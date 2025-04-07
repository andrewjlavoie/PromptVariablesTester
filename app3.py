import streamlit as st
import os
import re
import json
import datetime
import xml.dom.minidom as md
import xml.etree.ElementTree as ET
from typing import Dict, List, Any, Tuple, Optional, Union
import anthropic
from pathlib import Path

def parse_variable_definitions(var_definitions: str) -> Dict[str, Any]:
    """
    Parse variable definitions from the input string.
    
    Args:
        var_definitions: A string containing variable definitions in the format:
                       variable_name="value", file_var=$$file(path), dir_var=$$dir(path)
    
    Returns:
        A dictionary mapping variable names to their values or special handlers
    """
    variables = {}
    # Match variable definitions using regex
    pattern = r'([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^,]+)(?:,|$)'
    matches = re.finditer(pattern, var_definitions)
    
    for match in matches:
        var_name = match.group(1).strip()
        var_value = match.group(2).strip()
        
        # Handle special variable definitions with $$
        if var_value.startswith('$$'):
            if var_value.startswith('$$file(') and var_value.endswith(')'):
                # Extract file path
                file_path = var_value[7:-1].strip()
                variables[var_name] = {'type': 'file', 'path': file_path}
            
            elif var_value.startswith('$$dir(') and var_value.endswith(')'):
                # Extract directory path and check for recursive flag
                dir_content = var_value[6:-1].strip()
                if ',' in dir_content:
                    dir_path, params = dir_content.split(',', 1)
                    dir_path = dir_path.strip()
                    recursive = 'recursive=True' in params
                else:
                    dir_path = dir_content
                    recursive = False
                
                variables[var_name] = {'type': 'dir', 'path': dir_path, 'recursive': recursive}
            
            elif var_value.startswith('$$list(') and var_value.endswith(')'):
                # Extract list elements
                list_content = var_value[7:-1].strip()
                # Parse list content safely
                try:
                    # Use json to parse the list safely
                    list_elements = json.loads(list_content)
                    if not isinstance(list_elements, list):
                        list_elements = [list_content]
                except json.JSONDecodeError:
                    # Fall back to simple splitting by comma if json parsing fails
                    list_elements = [elem.strip() for elem in list_content.split(',')]
                
                variables[var_name] = {'type': 'list', 'elements': list_elements}
        else:
            # Handle regular variable definitions (strip quotes if present)
            if (var_value.startswith('"') and var_value.endswith('"')) or \
               (var_value.startswith("'") and var_value.endswith("'")):
                var_value = var_value[1:-1]
            
            variables[var_name] = var_value
    
    return variables

def expand_variables(variables: Dict[str, Any]) -> List[Dict[str, str]]:
    """
    Expand iterative variables into all possible combinations.
    
    Args:
        variables: Dictionary of parsed variables
    
    Returns:
        List of dictionaries, each containing a specific combination of variable values
    """
    # First, collect all iterative variables and their values
    iterative_vars = {}
    fixed_vars = {}
    
    for var_name, var_value in variables.items():
        if isinstance(var_value, dict):
            if var_value['type'] == 'file':
                # Single file - read content
                try:
                    with open(var_value['path'], 'r') as file:
                        fixed_vars[var_name] = file.read()
                except Exception as e:
                    st.error(f"Error reading file {var_value['path']}: {e}")
                    fixed_vars[var_name] = f"ERROR: Could not read file {var_value['path']}"
            
            elif var_value['type'] == 'dir':
                # Directory of files - iterative
                file_paths = []
                base_path = var_value['path']
                
                if var_value['recursive']:
                    # Recursively walk through directories
                    for root, _, files in os.walk(base_path):
                        for file in files:
                            file_paths.append(os.path.join(root, file))
                else:
                    # Only files in the top directory
                    if os.path.exists(base_path) and os.path.isdir(base_path):
                        file_paths = [os.path.join(base_path, f) for f in os.listdir(base_path) 
                                     if os.path.isfile(os.path.join(base_path, f))]
                
                # Read content of each file
                file_contents = []
                file_path_display = []  # Keep track of file paths for display
                for path in file_paths:
                    try:
                        with open(path, 'r') as file:
                            file_contents.append(file.read())
                            file_path_display.append(path)
                    except Exception as e:
                        st.warning(f"Skipping file {path}: {e}")
                
                if file_contents:
                    iterative_vars[var_name] = {"values": file_contents, "paths": file_path_display}
                else:
                    st.warning(f"No readable files found in directory: {base_path}")
                    fixed_vars[var_name] = f"No files found in {base_path}"
            
            elif var_value['type'] == 'list':
                # List of elements - iterative
                iterative_vars[var_name] = {"values": var_value['elements'], "paths": var_value['elements']}
        else:
            # Regular variable - fixed
            fixed_vars[var_name] = var_value
    
    # Generate all combinations of iterative variables
    combinations = [{}]
    
    for var_name, var_data in iterative_vars.items():
        new_combinations = []
        for combo in combinations:
            for i, value in enumerate(var_data["values"]):
                new_combo = combo.copy()
                new_combo[var_name] = value
                new_combo[f"{var_name}_path"] = var_data["paths"][i]
                new_combinations.append(new_combo)
        combinations = new_combinations
    
    # Add fixed variables to all combinations
    for combo in combinations:
        for var_name, value in fixed_vars.items():
            combo[var_name] = value
    
    return combinations

def render_template(template: str, variables: Dict[str, str]) -> str:
    """
    Replace template variables with their values.
    
    Args:
        template: The template string with {{variable}} placeholders
        variables: Dictionary of variable names and their values
    
    Returns:
        Rendered template with variables replaced
    """
    result = template
    for var_name, value in variables.items():
        if not var_name.endswith("_path"):  # Skip the path tracking variables
            placeholder = "{{" + var_name + "}}"
            result = result.replace(placeholder, str(value))
    
    # Check if any placeholders remain
    remaining_vars = re.findall(r'{{([^}]+)}}', result)
    if remaining_vars:
        for var in remaining_vars:
            result = result.replace("{{" + var + "}}", f"UNDEFINED_VARIABLE_{var}")
    
    return result

def call_llm(prompt: str, llm_params: Dict[str, Any]) -> str:
    """
    Call the LLM with the given prompt and parameters.
    
    Args:
        prompt: The prompt to send to the LLM
        llm_params: Dictionary of LLM parameters (temperature, etc.)
    
    Returns:
        The LLM's response
    """
    try:
        client = anthropic.Anthropic(api_key=llm_params.get("api_key", ""))
        
        message = client.messages.create(
            model=llm_params.get("model", "claude-3-opus-20240229"),
            max_tokens=llm_params.get("max_tokens", 1024),
            temperature=llm_params.get("temperature", 0.7),
            top_p=llm_params.get("top_p", 1.0),
            system=llm_params.get("system_prompt", ""),
            messages=[
                {"role": "user", "content": prompt}
            ]
        )
        
        return message.content[0].text
    except Exception as e:
        return f"Error calling LLM: {str(e)}"

def get_log_filename(base_path: str, append_datetime: bool) -> str:
    """
    Generate a log filename, optionally appending the current datetime.
    
    Args:
        base_path: The base path for the log file
        append_datetime: Whether to append the current datetime to the filename
        
    Returns:
        The final log file path
    """
    if not append_datetime:
        return base_path
        
    # Split the path into directory and filename
    directory = os.path.dirname(base_path)
    filename = os.path.basename(base_path)
    
    # Split filename into name and extension
    if '.' in filename:
        name, ext = filename.rsplit('.', 1)
        datetime_str = datetime.datetime.now().strftime("%d%b%Y_%H-%M-%S")
        new_filename = f"{name}_{datetime_str}.{ext}"
    else:
        datetime_str = datetime.datetime.now().strftime("%d%b%Y_%H-%M-%S")
        new_filename = f"{filename}_{datetime_str}"
    
    # Combine directory and new filename
    return os.path.join(directory, new_filename)

def log_session(log_file: str, session_data: Dict[str, Any]):
    """
    Log the session data to an XML file.
    
    Args:
        log_file: Path to the log file
        session_data: Dictionary containing session information
    """
    # Create root element for the session
    timestamp = datetime.datetime.now().strftime("%d%b%Y - %H:%M:%S")
    
    # Check if the file exists
    try:
        if os.path.exists(log_file):
            # Parse existing file
            tree = ET.parse(log_file)
            root = tree.getroot()
        else:
            # Create new root element
            root = ET.Element("sessions")
    except Exception as e:
        st.warning(f"Error opening log file: {e}. Creating new log.")
        root = ET.Element("sessions")
    
    # Create session element
    session = ET.SubElement(root, "session")
    session.set("datetime", timestamp)
    
    # Add each input
    for i, input_data in enumerate(session_data["inputs"]):
        input_elem = ET.SubElement(session, f"input{i+1}")
        
        # Add variables
        vars_elem = ET.SubElement(input_elem, "variables")
        # Add variable information
        for var_name, var_value in input_data["variables"].items():
            if var_name.endswith("_path"):
                var_elem = ET.SubElement(vars_elem, var_name)
                var_elem.text = str(var_value)
        
        # Add LLM parameters
        params_elem = ET.SubElement(vars_elem, "llm_parameters")
        for param_name, param_value in session_data["llm_params"].items():
            if param_name != "api_key":  # Skip API key for security
                param = ET.SubElement(params_elem, param_name)
                param.text = str(param_value)
        
        # Add prompt
        prompt_elem = ET.SubElement(input_elem, "prompt")
        prompt_elem.text = input_data["prompt"]
        
        # Add output
        output_elem = ET.SubElement(input_elem, "output")
        output_elem.text = input_data["output"]
    
    # Write to file with pretty formatting
    tree = ET.ElementTree(root)
    xml_str = ET.tostring(root, encoding='utf-8')
    pretty_xml = md.parseString(xml_str).toprettyxml(indent="  ")
    
    # Create directory if it doesn't exist
    os.makedirs(os.path.dirname(log_file), exist_ok=True)
    
    with open(log_file, "w", encoding="utf-8") as f:
        f.write(pretty_xml)

def main():
    """
    Main Streamlit application function.
    """
    st.title("Templated Prompt Tester")
    
    # Create sidebar for LLM parameters
    st.sidebar.header("LLM Parameters")
    
    api_key = st.sidebar.text_input("API Key", type="password")
    model = st.sidebar.selectbox(
        "Model", 
        ["claude-3-opus-20240229", "claude-3-sonnet-20240229", "claude-3-haiku-20240307"]
    )
    temperature = st.sidebar.slider("Temperature", 0.0, 1.0, 0.7)
    max_tokens = st.sidebar.number_input("Max Tokens", 1, 4096, 1024)
    top_p = st.sidebar.slider("Top P", 0.0, 1.0, 1.0)
    system_prompt = st.sidebar.text_area("System Prompt", "")
    
    st.sidebar.markdown("---")
    st.sidebar.header("Application Settings")
    max_iterations = st.sidebar.number_input("Max Iterations", 1, 100, 10, 
                                           help="Maximum number of combinations to process")
    log_file = st.sidebar.text_input("Log File Path", "logs/prompt_tests.xml")
    append_datetime = st.sidebar.checkbox("Append datetime to log filename", value=True,
                                     help="Add current date and time to the log file name")
    
    # Main interface
    prompt_template = st.text_area("Prompt Template", 
                                 "Write a short summary of {{file_content}}.", 
                                 height=200)
    
    # Variable definitions input
    st.subheader("Variable Definitions")
    st.markdown("""
    Format: `variable_name="value", file_var=$$file(path), dir_var=$$dir(path), list_var=$$list([elem1, elem2])`
    
    Special variable types:
    - `$$file(path)` - Content of a single file
    - `$$dir(path)` - Content of all files in a directory (non-recursive)
    - `$$dir(path, recursive=True)` - Content of all files in a directory (recursive)
    - `$$list([elem1, elem2, ...])` - List of elements
    """)
    
    var_definitions = st.text_area("Variable Definitions", 
                                "file_content=$$dir(./sample_data)")
    
    if st.button("Test Prompt"):
        # Check for API key
        if not api_key:
            st.error("Please enter your API key in the sidebar.")
            return
        
        with st.spinner("Processing..."):
            # Parse variable definitions
            variables = parse_variable_definitions(var_definitions)
            
            # Expand variables to all combinations
            combinations = expand_variables(variables)
            
            if not combinations:
                st.error("No valid combinations found. Please check your variable definitions.")
                return
                
            # Limit the number of combinations to process
            if len(combinations) > max_iterations:
                st.warning(f"Found {len(combinations)} possible combinations. Limiting to {max_iterations} as configured.")
                combinations = combinations[:max_iterations]
            
            # Prepare session data for logging
            session_data = {
                "llm_params": {
                    "api_key": api_key,
                    "model": model,
                    "temperature": temperature,
                    "max_tokens": max_tokens,
                    "top_p": top_p,
                    "system_prompt": system_prompt,
                    "max_iterations": max_iterations
                },
                "inputs": []
            }
            
            # Process each combination
            for i, combo in enumerate(combinations):
                # Display variable combination (for files, show path instead of content)
                display_vars = {}
                for var_name, var_value in combo.items():
                    if var_name.endswith("_path"):
                        display_vars[var_name] = var_value
                    else:
                        # For display purposes, truncate large content
                        if isinstance(var_value, str) and len(var_value) > 100:
                            display_vars[var_name] = var_value[:100] + "..."
                        else:
                            display_vars[var_name] = var_value
                
                st.subheader(f"Combination {i+1}")
                st.write("Variables:")
                st.json(display_vars)
                
                # Render the template
                rendered_prompt = render_template(prompt_template, combo)
                
                # Display the rendered template
                with st.expander("Rendered Prompt"):
                    st.text_area("", rendered_prompt, height=150)
                
                # Call the LLM
                llm_params = {
                    "api_key": api_key,
                    "model": model,
                    "temperature": temperature,
                    "max_tokens": max_tokens,
                    "top_p": top_p,
                    "system_prompt": system_prompt
                }
                
                response = call_llm(rendered_prompt, llm_params)
                
                # Display the LLM's response
                st.write("LLM Response:")
                st.text_area("", response, height=200)
                
                # Add to session data for logging
                session_data["inputs"].append({
                    "variables": combo,
                    "prompt": rendered_prompt,
                    "output": response
                })
            
            # Log the session
            try:
                final_log_file = get_log_filename(log_file, append_datetime)
                log_session(final_log_file, session_data)
                st.success(f"Session logged to {final_log_file}")
            except Exception as e:
                st.error(f"Error logging session: {e}")
            
            st.balloons()

if __name__ == "__main__":
    main()
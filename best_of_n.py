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
                
            elif var_value.startswith('$$results(') and var_value.endswith(')'):
                # Extract results variable name
                results_var_name = var_value[10:-1].strip()
                variables[var_name] = {'type': 'results', 'var_name': results_var_name}
                
            elif var_value.startswith('$$initial_prompt(') and var_value.endswith(')'):
                # Special variable type for the initial prompt
                variables[var_name] = {'type': 'initial_prompt'}
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
            
            elif var_value['type'] == 'results':
                # Results variable - will be populated later
                fixed_vars[var_name] = {'type': 'results', 'var_name': var_value['var_name']}
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
        if not var_name.endswith("_path") and not isinstance(value, dict):  # Skip the path tracking variables and special types
            placeholder = "{{" + var_name + "}}"
            result = result.replace(placeholder, str(value))
    
    # Check if any placeholders remain
    remaining_vars = re.findall(r'{{([^}]+)}}', result)
    if remaining_vars:
        for var in remaining_vars:
            result = result.replace("{{" + var + "}}", f"UNDEFINED_VARIABLE_{var}")
    
    return result

def process_special_variables(template: str, variables: Dict[str, Any], initial_prompt: str, outputs: List[str]) -> str:
    """
    Process special variables in the template, like results and initial_prompt.
    
    Args:
        template: The template string with variables
        variables: Dictionary of variable definitions
        initial_prompt: The rendered initial prompt
        outputs: List of LLM outputs from the first prompt
    
    Returns:
        Template with special variables replaced
    """
    result = template
    
    for var_name, var_value in variables.items():
        if isinstance(var_value, dict):
            if var_value.get('type') == 'results':
                # Extract the variable name to use in XML tags
                xml_var_name = var_value.get('var_name')
                
                # Build the XML string for all outputs
                xml_outputs = ""
                for i, output in enumerate(outputs, 1):
                    xml_outputs += f"<{xml_var_name}{i}>\n{output}\n</{xml_var_name}{i}>\n"
                
                # Replace the placeholder with the XML string
                placeholder = "{{" + var_name + "}}"
                result = result.replace(placeholder, xml_outputs)
            
            elif var_value.get('type') == 'initial_prompt':
                # Replace the initial_prompt placeholder
                placeholder = "{{" + var_name + "}}"
                result = result.replace(placeholder, initial_prompt)
    
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
            model=llm_params.get("model", "claude-3-7-sonnet-latest"),
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

def batch_call_llm(prompt: str, llm_params: Dict[str, Any], num_runs: int) -> List[str]:
    """
    Make batch calls to the LLM with the same prompt multiple times.
    
    Args:
        prompt: The prompt to send to the LLM
        llm_params: Dictionary of LLM parameters (temperature, etc.)
        num_runs: Number of times to run the same prompt
    
    Returns:
        List of LLM responses
    """
    try:
        client = anthropic.Anthropic(api_key=llm_params.get("api_key", ""))
        
        # Create batch of identical messages
        batch_messages = []
        for _ in range(num_runs):
            batch_messages.append({
                "model": llm_params.get("model", "claude-3-7-sonnet-latest"),
                "max_tokens": llm_params.get("max_tokens", 1024),
                "temperature": llm_params.get("temperature", 0.7),
                "top_p": llm_params.get("top_p", 1.0),
                "system": llm_params.get("system_prompt", ""),
                "messages": [
                    {"role": "user", "content": prompt}
                ]
            })
        
        # Submit batch request
        batch_results = []
        for message_params in batch_messages:
            message = client.messages.create(**message_params)
            batch_results.append(message.content[0].text)
        
        return batch_results
    except Exception as e:
        return [f"Error calling LLM: {str(e)}"] * num_runs

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
    
    # Add initial prompt section
    initial_section = ET.SubElement(session, "initial_prompt")
    initial_section.set("num_runs", str(session_data["num_runs"]))
    
    # Add LLM parameters
    params_elem = ET.SubElement(initial_section, "llm_parameters")
    for param_name, param_value in session_data["llm_params"].items():
        if param_name != "api_key":  # Skip API key for security
            param = ET.SubElement(params_elem, param_name)
            param.text = str(param_value)
    
    # Add prompt template
    prompt_template = ET.SubElement(initial_section, "prompt_template")
    prompt_template.text = session_data["prompt_template"]
    
    # Add variables
    vars_elem = ET.SubElement(initial_section, "variables")
    for var_name, var_def in session_data["variables"].items():
        var_elem = ET.SubElement(vars_elem, var_name)
        if isinstance(var_def, dict):
            var_elem.set("type", var_def.get("type", ""))
            if "path" in var_def:
                var_elem.set("path", var_def["path"])
            if "recursive" in var_def:
                var_elem.set("recursive", str(var_def["recursive"]))
        else:
            var_elem.text = str(var_def)
    
    # Add each run
    runs_section = ET.SubElement(initial_section, "runs")
    for i, run_data in enumerate(session_data["runs"]):
        run_elem = ET.SubElement(runs_section, f"run{i+1}")
        
        # Add rendered prompt
        prompt_elem = ET.SubElement(run_elem, "rendered_prompt")
        prompt_elem.text = run_data["prompt"]
        
        # Add output
        output_elem = ET.SubElement(run_elem, "output")
        output_elem.text = run_data["output"]
    
    # Add evaluation section
    eval_section = ET.SubElement(session, "evaluation")
    
    # Add prompt template
    eval_prompt_template = ET.SubElement(eval_section, "prompt_template")
    eval_prompt_template.text = session_data["eval_prompt_template"]
    
    # Add variables
    eval_vars_elem = ET.SubElement(eval_section, "variables")
    for var_name, var_def in session_data["eval_variables"].items():
        var_elem = ET.SubElement(eval_vars_elem, var_name)
        if isinstance(var_def, dict):
            var_elem.set("type", var_def.get("type", ""))
            if "var_name" in var_def:
                var_elem.set("var_name", var_def["var_name"])
        else:
            var_elem.text = str(var_def)
    
    # Add rendered prompt
    eval_prompt_elem = ET.SubElement(eval_section, "rendered_prompt")
    eval_prompt_elem.text = session_data["eval_rendered_prompt"]
    
    # Add final output
    eval_output_elem = ET.SubElement(eval_section, "output")
    eval_output_elem.text = session_data["eval_output"]
    
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
    st.title("Best of N Prompt Evaluator")
    
    # Create sidebar for LLM parameters
    st.sidebar.header("LLM Parameters")
    
    api_key = st.sidebar.text_input("API Key", type="password")
    model = st.sidebar.selectbox(
        "Model", 
        ["claude-3-7-sonnet-latest", "claude-3-5-haiku-latest", "claude-3-haiku-20240307"]
    )
    temperature = st.sidebar.slider("Temperature", 0.0, 1.0, 0.7)
    max_tokens = st.sidebar.number_input("Max Tokens", 1, 128000, 32000)
    top_p = st.sidebar.slider("Top P", 0.0, 1.0, 1.0)
    system_prompt = st.sidebar.text_area("System Prompt", "")
    
    st.sidebar.markdown("---")
    st.sidebar.header("Best of N Settings")
    num_runs = st.sidebar.number_input("Number of Runs (N)", 2, 20, 5, 
                                      help="Number of times to run the initial prompt")
    
    st.sidebar.markdown("---")
    st.sidebar.header("Application Settings")
    log_file = st.sidebar.text_input("Log File Path", "logs/best_of_n_tests.xml")
    append_datetime = st.sidebar.checkbox("Append datetime to log filename", value=True,
                                     help="Add current date and time to the log file name")
    
    # Main input section (no tabs for inputs)
    st.header("Initial Prompt")
    initial_prompt_template = st.text_area(
        "Prompt Template", 
        "Generate a creative solution to the following problem: {{problem}}", 
        height=200
    )
    
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
    
    initial_var_definitions = st.text_area(
        "Variable Definitions", 
        'problem="Design a system to reduce water waste in urban environments"'
    )

    st.markdown("---")
    
    st.header("Evaluation Prompt")
    eval_prompt_template = st.text_area(
        "Evaluation Prompt Template", 
        """For the following input prompt, evaluate the outputs and create a single output that takes the strengths of each into one best output:

<InputPrompt>{{initial_prompt}}</InputPrompt>

{{outputs}}
""", 
        height=200
    )
    
    # Variable definitions input
    st.subheader("Variable Definitions")
    st.markdown("""
    Format: `variable_name="value", results_var=$$results(xml_tag_name), initial_prompt=$$initial_prompt()`
    
    Special variable types:
    - `$$results(xml_tag_name)` - Outputs from the initial prompt runs, wrapped in XML tags
    - `$$initial_prompt()` - The rendered initial prompt
    """)
    
    eval_var_definitions = st.text_area(
        "Variable Definitions", 
        "outputs=$$results(Output), initial_prompt=$$initial_prompt()"
    )
    
    # Results section (will be populated after running)
    st.markdown("---")
    
    results_container = st.container()
    with results_container:
        st.header("Results will appear here after running")
    
    # Run button
    if st.button("Run Best of N Evaluation"):
        # Check for API key
        if not api_key:
            st.error("Please enter your API key in the sidebar.")
            return
        
        with st.spinner("Processing initial prompt runs..."):
            # Parse variable definitions for initial prompt
            initial_variables = parse_variable_definitions(initial_var_definitions)
            
            # Expand variables to all combinations
            initial_combinations = expand_variables(initial_variables)
            
            if not initial_combinations:
                st.error("No valid combinations found for initial prompt. Please check your variable definitions.")
                return
            
            # Prepare LLM parameters
            llm_params = {
                "api_key": api_key,
                "model": model,
                "temperature": temperature,
                "max_tokens": max_tokens,
                "top_p": top_p,
                "system_prompt": system_prompt
            }
            
            # Store outputs for each run
            run_data = []
            
            # Process first combination only (for now)
            combo = initial_combinations[0]
            
            # Render the template
            rendered_prompt = render_template(initial_prompt_template, combo)
            
            # Run the prompt N times using batch call
            status_text = st.empty()
            run_progress = st.progress(0)
            
            status_text.text(f"Running {num_runs} iterations in batch...")
            run_progress.progress(0.1)
            
            # Use batch call for all runs
            outputs = batch_call_llm(rendered_prompt, llm_params, num_runs)
            
            # Store run data
            run_data = []
            for output in outputs:
                run_data.append({
                    "prompt": rendered_prompt,
                    "output": output
                })
            
            status_text.text("Initial runs completed!")
            run_progress.progress(1.0)
        
        # Now handle the evaluation prompt
        with st.spinner("Processing evaluation prompt..."):
            # Parse variable definitions for evaluation prompt
            eval_variables = parse_variable_definitions(eval_var_definitions)
            
            # Process special variables (results and initial_prompt)
            eval_rendered_prompt = process_special_variables(eval_prompt_template, eval_variables, rendered_prompt, outputs)
            
            # Call the LLM for evaluation
            eval_response = call_llm(eval_rendered_prompt, llm_params)
            
            # Prepare session data for logging
            session_data = {
                "llm_params": llm_params,
                "num_runs": num_runs,
                "prompt_template": initial_prompt_template,
                "variables": initial_variables,
                "runs": run_data,
                "eval_prompt_template": eval_prompt_template,
                "eval_variables": eval_variables,
                "eval_rendered_prompt": eval_rendered_prompt,
                "eval_output": eval_response
            }
            
            # Log the session
            try:
                final_log_file = get_log_filename(log_file, append_datetime)
                log_session(final_log_file, session_data)
                st.success(f"Session logged to {final_log_file}")
            except Exception as e:
                st.error(f"Error logging session: {e}")
        
        # Update the Results section
        results_container.empty()
        with results_container:
            st.header("Results")
            
            st.subheader("Initial Prompt")
            st.write(rendered_prompt)
            
            st.subheader("Individual Runs")
            # Create tabs only for the outputs
            run_tabs = st.tabs([f"Run {i+1}" for i in range(num_runs)])
            for i, (tab, output) in enumerate(zip(run_tabs, outputs)):
                with tab:
                    st.text_area(f"Output {i+1}", output, height=200)
            
            st.subheader("Evaluation Prompt")
            with st.expander("Show Full Evaluation Prompt"):
                st.text_area("", eval_rendered_prompt, height=200)
            
            st.subheader("Final Best of N Result")
            st.text_area("Final Output", eval_response, height=300)
            
            st.balloons()

if __name__ == "__main__":
    main()
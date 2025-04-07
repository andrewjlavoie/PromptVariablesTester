# Templated Prompt Tester

A Streamlit application for testing templated prompts against multiple files and variables.

## Features

- Test prompt templates with variable substitution
- Process multiple files in directories (with recursive option)
- Custom variable definitions with special file handlers
- LLM parameter configuration
- Detailed logging of all inputs and outputs

## Installation

1. Clone this repository
2. Install the required dependencies:

```bash
pip install streamlit anthropic
```

3. Run the application:

```bash
streamlit run app.py
```

## Usage

### Creating Templates

Create prompt templates using double curly braces for variables:

```
Write a summary of the following document: {{file_content}}

Compare these two files: {{file1}} and {{file2}}
```

### Variable Definitions

Define variables in the following format:

```
variable_name="value", file_var=$$file(path), dir_var=$$dir(path)
```

Special variable types:

- `$$file(path)` - Content of a single file
- `$$dir(path)` - Content of all files in a directory (non-recursive)
- `$$dir(path, recursive=True)` - Content of all files in a directory (recursive)
- `$$list([elem1, elem2, ...])` - List of elements

Examples:

```
# Basic variable
name="John Doe"

# File variable
document=$$file(/path/to/file.txt)

# Directory of files (non-recursive)
documents=$$dir(/path/to/docs)

# Directory of files (recursive)
all_documents=$$dir(/path/to/docs, recursive=True)

# List of elements
options=$$list(["option1", "option2", "option3"])
```

### LLM Parameters

Configure LLM parameters in the sidebar:

- API Key: Your Anthropic API key
- Model: Select the Claude model to use
- Temperature: Controls randomness (0.0-1.0)
- Max Tokens: Maximum length of responses
- Top P: Nucleus sampling parameter
- System Prompt: System prompt for the LLM

### Application Settings

- Max Iterations: Limits the number of combinations processed (default: 10)
- Log File Path: Where to save the session logs
- Append datetime to log filename: Automatically adds the current date and time to the log filename (enabled by default)

### Logging

All inputs and outputs are logged to the specified XML file in the following format:

```xml
<sessions>
  <session datetime="06Apr2025 - 10:30:45">
    <input1>
      <variables>
        <file_content_path>/path/to/file.txt</file_content_path>
        <llm_parameters>
          <model>claude-3-opus-20240229</model>
          <temperature>0.7</temperature>
          <!-- Other parameters -->
        </llm_parameters>
      </variables>
      <prompt>Write a summary of the following document: [file content here]</prompt>
      <output>[LLM response here]</output>
    </input1>
    <!-- Additional inputs -->
  </session>
  <!-- Additional sessions -->
</sessions>
```

## Example

1. Enter a prompt template:
   ```
   Write a summary of {{document}} in the style of {{style}}.
   ```

2. Define variables:
   ```
   document=$$dir(./data), style=$$list(["academic", "casual", "business"])
   ```

3. Configure LLM parameters in the sidebar

4. Click "Test Prompt" to process all combinations

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License.
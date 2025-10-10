# TermChat

Integrated A.I Chatbot inside the terminal. Hand-written entirely in C.

## Prerequisites

You must have the following for the project to work:

- Any Linux distribution
- Curl installed via your package manager
- OpenAI API-Key and enough credits to make requests

## Build

These were the resources used to develop this application. The project may still
compile and run under different environments, but this has not been tested nor
do I guarantee that it will work.

| Component | Description | Version |
| --------- | ----------- | ------- |
| C         | Language    | C23     |
| GCC       | Compiler\*  | 15.2.1  |
| Clang     | Compiler\*  | 20.1.8  |
| Curl      | Library     | 8.16.0  |

_\* GCC and Clang both supported. Choose the one you are most comfortable with. You maybe change it inside the `build.c` file._

Build the project using either of the following commands:

```bash
gcc build.c && ./a.out && rm a.out
```

```bash
clang -std=gnu23 build.c && ./a.out && rm a.out
```

A new folder `build` will be generated in the root of the project.

```bash
📂 /
├── 📂 build
    └── out
├── 📂 include
├── 📂 src
└── ...
```

Now you can simply execute the `build/out` binary:

```bash
./build/out "Hi, how are you?"
```

## Usage

Create this configuration file `~/.config/termchatrc.json`:

```json
{
  "model": "gpt-4.1",
  "role": "developer",
  "instruction": "Professional C Programmer",
  "openai": "<API-Key Here>"
}
```

### Normal mode

Use this mode to get a one time response looking like this:

```bash
./termchat "Who created the C programming language?"
```

```
+-------------------------------------------------------------------------------------------+
| gpt-4.1                                                                                   |
+-------------------------------------------------------------------------------------------+
| The C programming language was created by Dennis Ritchie at Bell Labs in the early 1970s. |
+-------------------------------------------------------------------------------------------+
```

### Interactive mode

With interactive mode, the LLM will remember the context of your conversation.
Pass the flag `-i` to activate it.

```bash
./termchat -i
```

```
+-------------------------------------------------------------------------------------------+
| gpt-4.1                                                                                   |
+-------------------------------------------------------------------------------------------+
| The C programming language was created by Dennis Ritchie at Bell Labs in the early 1970s. |
+-------------------------------------------------------------------------------------------+
(gpt-4.1)> Who created the C programming language?
```

### Command mode

You may configure the program to execute commands between backticks in the `~/.config/termchatrc.json`.
The program will then ask you if you would like to execute the command that the LLM
provided. Make sure you know what you are doing. Never blindly trust an LLM.

## Flags

The following flags can be used when starting the program.

| Short-form | Long-form     | Purpose                                  |
| ---------- | ------------- | ---------------------------------------- |
| -i         | --interactive | Starts the program in interactive mode   |
| -h         | --help        | Shows a table with all flags and options |

## Acknowledgements

Many thanks to these resources:

- [Libcurl](https://curl.se/)

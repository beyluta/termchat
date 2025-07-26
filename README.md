# TermChat

Integrated A.I Chatbot inside the terminal. Programmed entirely in C.

## Prerequisites

Make sure to have the following at hand:

- Any Linux operating system
- Curl installed via your package manager
- OpenAI API-key any enough credits to make requests

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

After this is done, run the binary like this:

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

## Flags

The following flags can be used when starting the program.

| Short-form | Long-form     | Purpose                                  |
| ---------- | ------------- | ---------------------------------------- |
| -i         | --interactive | Starts the program in interactive mode   |
| -h         | --help        | Shows a table with all flags and options |

## Development

These were the resources used to develop this application. The project may still
compile and run under different environments, but this has not been tested nor
do I guarantee that it will work.

| Component | Description      | Version        |
| --------- | ---------------- | -------------- |
| C         | Language         | C23            |
| Clang     | Compiler         | 20.1.8         |
| GNU Make  | Build Automation | 4.4.1          |
| Linux     | Kernel           | 6.15.7-arch1-1 |
| Curl      | Library          | 8.15.0         |

Simply clone or fork the project then run the following
command to update dependencies and compile:

```bash
make build
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

## Acknowledgements

Many thanks to these resources:

- [Libcurl](https://curl.se/)
- [jsmn](https://github.com/zserge/jsmn)

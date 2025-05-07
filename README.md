# TermChat

Integrated A.I Chatbot inside the terminal. Programmed entirely in C.

## Prerequisites

Make sure to have the following at hand:

- Linux environment
- OpenAI API-key

For developers you also need the following:

- GNU Make >= 4.4.1
- clang >= 19.1.7

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

```bash
./termchat "Who created the C programming language?"
```

You will get a response looking like this:

```
+-------------------------------------------------------------------------------------------+
| gpt-4.1                                                                                   |
+-------------------------------------------------------------------------------------------+
| The C programming language was created by Dennis Ritchie at Bell Labs in the early 1970s. |
+-------------------------------------------------------------------------------------------+
```

## Flags

The following flags can be used when starting the program.

| Short-form | Long-form     | Purpose                                  |
| ---------- | ------------- | ---------------------------------------- |
| -i         | --interactive | Starts the program in interactive mode   |
| -h         | --help        | Shows a table with all flags and options |

## Development

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

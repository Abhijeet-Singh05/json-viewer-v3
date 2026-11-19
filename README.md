# JSON Viewer CLI (V3)

A command-line JSON viewer written in C. It uses the cJSON library to parse JSON files.

## Features

- Display JSON objects and arrays as an indented tree
- Validate JSON syntax
- Search for object keys recursively
- Search scalar values using a case-insensitive substring
- Navigate nested objects and arrays with simple paths
- Show JSON statistics (node counts and maximum depth)
- Optional terminal colors (disabled when output is redirected or `NO_COLOR` is set)

## Requirements (Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libcjson-dev  libcurl4-openssl-dev
```

## Build

Run these commands from the project root:

```bash
make
```

Or build manually:

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude \
    src/main.c src/parser.c src/viewer.c src/validator.c \
    -o jsonview -lcjson
```

## Run

```bash
./jsonview samples/sample.json
./jsonview --validate samples/sample.json
./jsonview --url url
./jsonview --stats samples/sample.json
./jsonview --find name samples/sample.json
./jsonview --filter delhi samples/sample.json
./jsonview --get address.city samples/sample.json
./jsonview --get 'skills[1]' samples/sample.json
./jsonview --get '$.education.year' samples/sample.json
./jsonview --download url destination
./jsonview --help
```

## Simple path syntax

Supported examples:

- `address.city`
- `skills[1]`
- `$.education.year`

This is a small path syntax, not a complete JSONPath implementation. Quoted object keys, wildcards, slices, and filter expressions are not supported.

## Project structure

```text
json-viewer-v3/
├── src/
│   ├── main.c
│   ├── parser.c
│   ├── viewer.c
│   └── validator.c
├── include/
│   ├── parser.h
│   ├── viewer.h
│   └── validator.h
├── samples/
│   └── sample.json
├── Makefile
└── README.md
```

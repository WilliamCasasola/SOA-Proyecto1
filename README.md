# SOA - Proyecto 1

## 1. Compile the program:
Navigate to the folder where the source files are located and execute:

```bash
$ cd to/project/path
$ make
```

## 2. Program execution and available options

Follow the next order: First create the creator, then the consumers and the producers. The finalizer stops everything.
You can use the `bulk` script to create multiple consumers or producers in one step (see 2.5).

## 2.1 Creator

### Command

```bash
$ ./creator <bufferName> [<bufferLength>]
```

Application receives up to two parameters, with the first being mandatory:

	 Buffer Name: char*
	 Buffer Length: +int

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<bufferName>`      | Name of the buffer  | char*    |           | No        |
| `<bufferLength>`      | Buffer capacity  | int   |       10        | yes        |



## 2.2 Producer

### Command

```bash
$ ./producer <bufferName> [<mean>]
```

Application receives up to two parameters, with the first being mandatory:

	 Buffer Name: char*
	 Producer Mean: +double

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<bufferName>`      | Name of the buffer  | char*    |           | No        |
| `<mean>`      | Mean for exponential time before produce to the buffer  |  double |       0.25        | yes        |


## 2.3 Consumer

### Command

```bash
$ ./consumer <bufferName> [<mean>]
```

Application receives up to two parameters, with the first being mandatory:

	 Buffer Name: char*
	 Consumer Mean: +double

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<bufferName>`      | Name of the buffer  | char*    |           | No        |
| `<mean>`      | Mean for exponential time before consume from the buffer  | double  |       0.25        | yes        |


## 2.4 Finalizer

### Command

```bash
$ ./finalizer <bufferName>
```

Application receives 1 parameter:

	 Buffer Name: char*

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<bufferName>`      | Name of the buffer  | char*    |           | No        |


## 2.5 bulk
This file allows you to create multiple consumers or producers in a single step:

### Command

```bash
$ ./bulk producer <number> <bufferName> [<mean>]
```

```bash
$ ./bulk consumer <number> <bufferName> [<mean>]
```

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<number>`      | Number of producers or consumers to be created | integer    |           | No        |
| `<bufferName>`      | Name of the buffer  | char*    |           | No        |
| `<mean>`      | Mean for exponential time before consume from the buffer  | double  |       0.25        | yes        |
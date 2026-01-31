# axgreet

axgreet is an experimental lightweight greetd greeter with no dependencies.

beware! author of this project is a JVM developer and doesn't know how to write C code. Don't use this, unless you really know what you're doing.

## Build

axgreet uses [nob](https://github.com/tsoding/nob.h) as build system.

### quick start:

prerequisites: git

```bash
$ ./nob
```

With nob, build instructions are stored as C code in `nob.c`.

### manual bootstrap

if you don't have git/curl/wget or operating in some other kind of wild environment, you can bootstrap nob yourself.

download [nob.h](https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h) to `build` folder, then run nob executable. E.g.:
```bash
$ mkdir build
$ wget https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h -o build/nob.h
$ ./nob
```

## Development and Contributing

You can run

```bash
$ ./nob test
```

to test greeter locally. That would start greetd with custom test config in tty2.

### Contribution

I will accept any contributions, as long as they are useful and in line with my vision of the project.

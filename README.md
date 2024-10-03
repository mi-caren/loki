# Loki text editor

A simple text editor written in C.

**Loki** is inspired by [antirez kilo](https://github.com/antirez/kilo) and its development started following the tutorial [Build your own text editor](https://viewsourcecode.org/snaptoken/kilo/).

It started as a learning project and aims to become a complete text editor for coding.

## Build
To build **Loki** you need to have `gcc` compiler installed

Build an optimized binary:
```shell
make release
```
---
Build a debug binary:
```shell
make
```

The created binary is called `loki`, you can then move it to a directory inside your `$PATH`

## Usage
```shell
loki <filename>
```
If a filename is not specified, **Loki** will create a new buffer.
### Keybindings
* **Ctrl-Q** -- _exit_
* **Ctrl-S** -- _save_
* **Ctrl-F** -- _search in current buffer_
* **Ctrl-N** -- _jump to next search result_
* **Ctrl-P** -- _jump to previous search result_
* **Ctrl-C** -- _copy_
* **Ctrl-V** -- _paste_

## Goals
* Being lightweight on resources
* Simple and intuitive, with common keybindings
* No configuration (see [Appendix](#no-configuration))
* Support for [LSP](https://en.wikipedia.org/wiki/Language_Server_Protocol) for code navigation, code completion and marking of warning/errors
* Support for git for diffs and blame
* Having fun programming it ([Appendix](#why-c))


## Acknowledgements
Thanks to [Salvatore Sanfilippo](https://github.com/antirez) for inspiring the development of **Loki**.

Thanks to [Paige Ruten](https://github.com/paigeruten) for the beautiful tutorial [Build your own text editor](https://viewsourcecode.org/snaptoken/kilo/).


## License
Licensed under MIT license.

## Appendix
### Reasons
The main reason I'm developing **Loki** is because I think that text editors are very cool peaces of software, and I thought it would be fun to learn how to develop one.
I don't think that the world needs another text editor, but I feel frustrated by modern, heavy and inefficient editors. Apart from the inefficiency, however, some of those text editors do their job (and sometimes they even do a lot more than they should), and the fact that they're resource-heavy seems to have become an acceptable trade-off in the modern world. But I like simple and minimal things, and it's really frustrating for me to use a software with a lot of useless functionality and a thousand way to configure it. Morover, depending on something very complex makes you believe that you need that complexity. **Loki** wants to be simple but functional, light but complete, a minimal and stress-free code editor. Last but not least, I want to have a lot of fun programming, maybe this is the most important reason for me to keep on developing it.

### No configuration
Being aware that this is just another text editor among many, and knowing that the world doesn't really need it, i know that probably I will be the only user of **Loki**. For this reason, I believe that having the possibility of configuring it, perhaps through a configuration file saved somewhere, would mean to introduce an unnecessary overhead. Maybe in the future I will introduce a way to configure it using a header file (Like [Suckless software](https://suckless.org/) does), but this is not one of my main goals.
### Why C?
As I said before, I like simple things, and C is a simple programming language that allows you to be very creative. I'm aware of the various risks that programming in C involves, but I can't stop using it. C makes me enjoy programming in a way that modern and safer programming languages can't. It forces me to think in a "low-level" way, and pushes me to find my own solutions to problems. Moreover, I like old things. They have charm, there is something ritual about them. They carry the legacy of past generations. I know it sounds like a paradox, but I feel _safe_ when I use them. And I feel cool when I use C.
# Neovim
This directory contains custom configuration files for Neovim.

## Requirements
- Neovim > 0.8
- Git

## Configuring
To configure Neovim follow these steps:

1. Copy all contents of this directory into the runtime path.
- `$XDG_CONFIG_HOME/nvim` in Linux
- `%LOCALAPPDATA%\nvim` in Windows

2. Install [packer](https://github.com/wbthomason/packer.nvim#requirements). This is the package manager for plugins.
3. Start Neovim and open the file `<RUNTIMEPATH>/lua/miguel/packer.lua`. Never mind any errors that you see for now.
4. Execute the command `:so` and then `:PackerSync` to install all the plugins.
5. Restart Neovim

And we are done! Start editing.

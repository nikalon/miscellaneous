# Neovim
This directory contains custom configuration files for Neovim.

## Requirements
- Neovim > 0.8
- Git
- In Windows install msvc (required for treesitter plugin) and [follow the instructions.](https://github.com/nvim-treesitter/nvim-treesitter#requirements)

## Configuring
To configure Neovim follow these steps:

1. Copy all contents of this directory into the runtime path.
- `$XDG_CONFIG_HOME/nvim` in Linux
- `%LOCALAPPDATA%\Local\nvim` in Windows

2. Install [packer](https://github.com/wbthomason/packer.nvim#requirements). This is the package manager for plugins.
3. Start Neovim and open the file `<RUNTIMEPATH>/lua/miguel/packer.lua`.
4. Execute the command `:so` and then `:PackerSync` to install all the plugins.

And we are done! Start editing.

# Neovim
This directory contains custom configuration files for Neovim. It's based on [kickstart](https://github.com/nvim-lua/kickstart.nvim).

## Requirements
- Neovim > 0.9.0

## Configuring
To configure Neovim follow these steps:

1. Copy init.lua into the runtime path
- `$XDG_CONFIG_HOME/nvim` in Linux
- `%LOCALAPPDATA%\nvim` in Windows

2. Install a C compiler for neovim-treesitter. I highly recommend to install `zig` in Widows because it's the easiest option and requires no setup.
- `winget install zig.zig` in Windows

3. Start Neovim and wait until the treesitters gets compiled. Restart Neovim again if needed.

And we are done! Start editing.

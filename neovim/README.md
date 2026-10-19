# Neovim
This directory contains custom configuration files for Neovim. It's based on [kickstart](https://github.com/nvim-lua/kickstart.nvim).

## Requirements
- Neovim >= 0.12.0 (plugins are installed with its built-in plugin manager, `vim.pack`)
- git

## Configuring
To configure Neovim follow these steps:

1. Copy init.lua into the runtime path
- Linux: `$XDG_CONFIG_HOME/nvim`
- Windows: `%LOCALAPPDATA%\nvim`

2. Enable integration with system clipboard.

In Ubuntu:
```bash
# If you're using X11
sudo apt install xclip

# If you're using Wayland
sudo apt install wl-clipboard
```

3. Start Neovim. The plugins get installed automatically on the first start.

That's all!

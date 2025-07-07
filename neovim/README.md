# Neovim
This directory contains custom configuration files for Neovim. It's based on [kickstart](https://github.com/nvim-lua/kickstart.nvim).

## Requirements
- Neovim > 0.9.0

## Configuring
To configure Neovim follow these steps:

1. Copy init.lua into the runtime path
Linux: `$XDG_CONFIG_HOME/nvim`
Windows: `%LOCALAPPDATA%\nvim`

2. Install a C compiler for neovim-treesitter. I highly recommend to install `zig` in Widows because it's the easiest option and requires no setup.

In Windows:
```cmd
winget install zig.zig`
```

3. Enable integration with system clipboard.

In Ubuntu:
```bash
# If you're using X11
sudo apt install xclip

# If you're using Wayland
sudo apt install wl-clipboard
```

4. Start Neovim and wait until the treesitters gets compiled. Restart Neovim again if needed.

That's all!

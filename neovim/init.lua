-- Neovim configuration. Originally based on kickstart.nvim (https://github.com/nvim-lua/kickstart.nvim).
-- Requires Neovim 0.12+ (for the built-in plugin manager, `:help vim.pack`) and git.

-- Set <space> as the leader key
-- See `:help mapleader`
--  NOTE: Must happen before plugins are loaded (otherwise wrong leader will be used)
vim.g.mapleader = ' '
vim.g.maplocalleader = ' '

-- [[ Plugins ]]
-- Installed with the built-in plugin manager on first start. See `:help vim.pack`
--  Update them with `:lua vim.pack.update()`
vim.pack.add({
  -- Theme inspired by Atom
  'https://github.com/navarasu/onedark.nvim',
  -- Fuzzy finder (files, buffers, grep). plenary is a library required by telescope.
  'https://github.com/nvim-lua/plenary.nvim',
  { src = 'https://github.com/nvim-telescope/telescope.nvim', version = '0.1.x' },
}, { confirm = false })

-- [[ Neovim configuration ]]

-- Set highlight on search
vim.o.hlsearch = false

-- Make line numbers default
vim.wo.number = true

-- Enable mouse mode
vim.o.mouse = 'a'

-- Sync clipboard between OS and Neovim.
--  Remove this option if you want your OS clipboard to remain independent.
--  See `:help 'clipboard'`
vim.o.clipboard = 'unnamedplus'

-- Enable break indent
vim.o.breakindent = true

-- Save undo history
vim.o.undofile = true

-- Case-insensitive searching UNLESS \C or capital in search
vim.o.ignorecase = true
vim.o.smartcase = true

-- Decrease update time
vim.o.updatetime = 250
vim.o.timeoutlen = 300

-- Set 4 spaces as tabs
vim.o.autoindent = false
vim.o.tabstop = 4
vim.o.shiftwidth = 4
vim.o.softtabstop = 4
vim.o.expandtab = true

vim.o.termguicolors = true
vim.opt.incsearch = true
vim.opt.scrolloff = 5
vim.opt.colorcolumn = "120"
vim.opt.linebreak = true
vim.opt.title = true
vim.opt.wrap = true
vim.opt.relativenumber = true


-- [[ Basic Keymaps ]]

-- Keymaps for better default experience
-- See `:help vim.keymap.set()`
vim.keymap.set({ 'n', 'v' }, '<Space>', '<Nop>', { silent = true })

-- Remap for dealing with word wrap
vim.keymap.set('n', 'k', "v:count == 0 ? 'gk' : 'k'", { expr = true, silent = true })
vim.keymap.set('n', 'j', "v:count == 0 ? 'gj' : 'j'", { expr = true, silent = true })
vim.keymap.set("v", "K", ":m '<-2<CR>gv=gv")
vim.keymap.set("v", "J", ":m '>+1<CR>gv=gv")
vim.keymap.set("i", "<C-;>", " {}<Esc>i\n<Esc>O")

-- Quicker way to add new lines above
vim.keymap.set({"n", "i"}, "<C-L>", "<Esc>O")

-- Insert the current date (YYYY-MM-DD) and time (HH:MM, 24h)
vim.keymap.set("i", "<C-f>", function() return os.date("%Y-%m-%d") end, { expr = true, desc = "Insert current date" })
vim.keymap.set("i", "<C-d>", function() return os.date("%H:%M") end, { expr = true, desc = "Insert current time" })

-- [[ Highlight on yank ]]
-- See `:help vim.hl.on_yank()`
local highlight_group = vim.api.nvim_create_augroup('YankHighlight', { clear = true })
vim.api.nvim_create_autocmd('TextYankPost', {
  callback = function()
    vim.hl.on_yank()
  end,
  group = highlight_group,
  pattern = '*',
})


-- [[ Configure theme ]]
require('onedark').setup {
    style = 'darker'
}
require('onedark').load()
vim.o.guifont = "Consolas:h12"

-- [[ Configure Telescope ]]
-- See `:help telescope` and `:help telescope.setup()`
require('telescope').setup {
  defaults = {
    mappings = {
      i = {
        ['<C-u>'] = false,
        ['<C-d>'] = false,
        ['<Esc>'] = "close"
      },
    },
  },
}

-- [[ Self-contained live grep ]]
-- Pure Lua replacement for telescope's live_grep, so ripgrep is not needed.
-- Files are walked and read through libuv (vim.uv), which behaves the same on
-- Linux and Windows. Matching is literal (no regex) and smart case: the search is
-- case-insensitive unless the prompt contains an uppercase letter.
local live_grep
do
  local uv = vim.uv

  local grep_config = {
    -- Directory names that are never searched.
    ignored_dirs = {
      ['.git'] = true, ['.svn'] = true, ['.hg'] = true, ['node_modules'] = true,
      ['__pycache__'] = true, ['.cache'] = true, ['.vs'] = true, ['.idea'] = true,
    },
    -- Skip files and directories whose name starts with a dot.
    skip_hidden = true,
    -- Files bigger than this (in bytes) are skipped.
    max_file_size = 1024 * 1024,
    -- Stop searching once this many matching lines have been found.
    max_results = 10000,
    -- Longest slice of a matching line shown in the results window.
    max_line_length = 200,
    -- Milliseconds of searching before handing control back to the editor, so typing stays responsive.
    time_slice_ms = 10,
  }

  -- Lazily walks `root`. `list.get(i)` returns the i-th file path (relative to `root`),
  -- scanning only as many directories as needed. The walk is shared by every search of a picker.
  local function new_file_list(root)
    local list = { files = {}, pending_dirs = { '' }, done = false }

    local function scan_dir(dir)
      local handle = uv.fs_scandir(dir == '' and root or root .. '/' .. dir)
      if not handle then
        return
      end

      local names, subdirs = {}, {}
      while true do
        local name, kind = uv.fs_scandir_next(handle)
        if not name then
          break
        end

        if not (grep_config.skip_hidden and name:sub(1, 1) == '.') then
          local path = dir == '' and name or dir .. '/' .. name
          local is_link = kind == 'link' or kind == nil
          if is_link then
            local stat = uv.fs_stat(root .. '/' .. path)
            kind = stat and stat.type
          end

          -- Symlinked directories are not followed to avoid cycles.
          if kind == 'directory' and not is_link and not grep_config.ignored_dirs[name] then
            table.insert(subdirs, path)
          elseif kind == 'file' then
            table.insert(names, path)
          end
        end
      end

      table.sort(names)
      table.sort(subdirs)
      for _, path in ipairs(names) do
        table.insert(list.files, path)
      end
      -- pending_dirs is a stack, so push in reverse to visit subdirectories alphabetically.
      for i = #subdirs, 1, -1 do
        table.insert(list.pending_dirs, subdirs[i])
      end
    end

    function list.get(index)
      while list.files[index] == nil and not list.done do
        local dir = table.remove(list.pending_dirs)
        if dir then
          scan_dir(dir)
        else
          list.done = true
        end
      end
      return list.files[index]
    end

    return list
  end

  -- Returns the file contents, or nil if it can't be read, is too big or looks binary.
  local function read_file(path)
    local fd = uv.fs_open(path, 'r', 438)
    if not fd then
      return nil
    end

    local stat = uv.fs_fstat(fd)
    local data
    if stat and stat.size <= grep_config.max_file_size then
      data = uv.fs_read(fd, stat.size, 0)
    end
    uv.fs_close(fd)

    if data and data:find('\0', 1, true) then
      return nil
    end
    return data
  end

  -- Calls `on_match(lnum, col, line)` for the first occurrence of `needle` on each line.
  -- `haystack` is `content` or its lowercase version (same length, so positions line up).
  -- Stops early when `on_match` returns true.
  local function search_content(content, haystack, needle, on_match)
    local lnum, line_start = 1, 1
    local pos = haystack:find(needle, 1, true)

    while pos do
      local line_end = content:find('\n', line_start, true)
      while line_end and line_end < pos do
        lnum = lnum + 1
        line_start = line_end + 1
        line_end = content:find('\n', line_start, true)
      end

      local line = content:sub(line_start, (line_end or #content + 1) - 1):gsub('\r$', '')
      if on_match(lnum, pos - line_start + 1, line) then
        return true
      end

      if not line_end then
        break
      end
      lnum = lnum + 1
      line_start = line_end + 1
      pos = haystack:find(needle, line_start, true)
    end
  end

  local function make_entry(cwd, file, lnum, col, line)
    local text = line
    if #text > grep_config.max_line_length then
      -- Show the part of a long line that contains the match.
      local first = math.max(1, col - math.floor(grep_config.max_line_length / 4))
      text = (first > 1 and '…' or '') .. text:sub(first, first + grep_config.max_line_length - 1)
    end

    local location = string.format('%s:%d:%d:', file, lnum, col)
    local value = location .. text
    return {
      value = value,
      ordinal = value,
      display = function()
        return value, { { { 0, #file }, 'TelescopeResultsIdentifier' }, { { #file, #location }, 'TelescopeResultsLineNr' } }
      end,
      filename = file,
      path = cwd .. '/' .. file,
      lnum = lnum,
      col = col,
      text = line,
    }
  end

  -- Telescope finder: called with the new prompt on every keystroke. It starts a search
  -- and returns right away; results are delivered through `process_result`.
  local function new_finder(cwd)
    local finder = {
      files = new_file_list(cwd),
      contents = {}, -- relative path -> file contents, or false if unreadable
      lowered = {}, -- relative path -> lowercase contents, filled on demand
      generation = 0,
    }

    local function get_contents(file, ignore_case)
      local content = finder.contents[file]
      if content == nil then
        content = read_file(cwd .. '/' .. file) or false
        finder.contents[file] = content
      end
      if not content or not ignore_case then
        return content, content
      end

      local lowered = finder.lowered[file]
      if not lowered then
        lowered = content:lower()
        finder.lowered[file] = lowered
      end
      return content, lowered
    end

    local function search(generation, prompt, process_result, process_complete)
      local ignore_case = not prompt:find('%u')
      local needle = ignore_case and prompt:lower() or prompt
      local slice_ns = grep_config.time_slice_ms * 1e6
      local slice_start = uv.hrtime()
      local count = 0
      local cancelled = false

      -- Yields when the current time slice is used up. Returns true if a newer search replaced this one.
      local function pause_if_needed()
        if uv.hrtime() - slice_start > slice_ns then
          coroutine.yield()
          slice_start = uv.hrtime()
        end
        cancelled = cancelled or generation ~= finder.generation
        return cancelled
      end

      local index = 1
      while not pause_if_needed() do
        local file = finder.files.get(index)
        if not file then
          break
        end
        index = index + 1

        local content, haystack = get_contents(file, ignore_case)
        if content then
          local stop = search_content(content, haystack, needle, function(lnum, col, line)
            count = count + 1
            -- process_result returns true once the picker no longer wants results.
            cancelled = process_result(make_entry(cwd, file, lnum, col, line)) == true
            return cancelled or count >= grep_config.max_results or pause_if_needed()
          end)
          if stop then
            break
          end
        end
      end

      if not cancelled then
        process_complete()
      end
    end

    -- Runs `fn` as a coroutine, resuming it on the next event loop iteration whenever it yields.
    -- A 0ms timer is used instead of vim.schedule: callbacks scheduled from a scheduled callback
    -- run in the same queue drain, which would starve keyboard input until the search ends.
    local function run_in_slices(fn)
      local co = coroutine.create(fn)
      local function step()
        local ok, err = coroutine.resume(co)
        if not ok then
          vim.notify('live_grep: ' .. tostring(err), vim.log.levels.ERROR)
        elseif coroutine.status(co) ~= 'dead' then
          vim.defer_fn(step, 0)
        end
      end
      vim.defer_fn(step, 0)
    end

    -- Called by telescope when the picker closes: stops any running search.
    function finder.close()
      finder.generation = finder.generation + 1
    end

    return setmetatable(finder, {
      __call = function(_, prompt, process_result, process_complete)
        -- Bumping the generation cancels the search started by the previous keystroke.
        finder.generation = finder.generation + 1
        if not prompt or prompt == '' then
          process_complete()
          return
        end

        local generation = finder.generation
        run_in_slices(function()
          search(generation, prompt, process_result, process_complete)
        end)
      end,
    })
  end

  -- Highlights every occurrence of the prompt in a results row, with the same smart case rule.
  local function new_sorter()
    return require('telescope.sorters').Sorter:new {
      scoring_function = function()
        return 1
      end,
      highlighter = function(_, prompt, display)
        if prompt == '' then
          return {}
        end
        local ignore_case = not prompt:find('%u')
        local haystack = ignore_case and display:lower() or display
        local needle = ignore_case and prompt:lower() or prompt

        local highlights = {}
        local start = haystack:find(needle, 1, true)
        while start do
          table.insert(highlights, { start = start, finish = start + #needle - 1 })
          start = haystack:find(needle, start + #needle, true)
        end
        return highlights
      end,
    }
  end

  live_grep = function(opts)
    opts = opts or {}
    local cwd = (opts.cwd or uv.cwd()):gsub('\\', '/'):gsub('(.)/$', '%1')

    require('telescope.pickers')
      .new(opts, {
        prompt_title = 'Live Grep',
        finder = new_finder(cwd),
        previewer = require('telescope.config').values.grep_previewer { cwd = cwd },
        sorter = new_sorter(),
      })
      :find()
  end
end

-- [[ Search keymaps ]]
-- See `:help telescope.builtin`
vim.keymap.set('n', '<leader>?', require('telescope.builtin').oldfiles, { desc = '[?] Find recently opened files' })
vim.keymap.set('n', '<leader>b', require('telescope.builtin').buffers, { desc = '[ ] Find existing buffers' })
vim.keymap.set('n', '<C-n>', require('telescope.builtin').find_files, { desc = '[S]earch [F]iles' })
vim.keymap.set('n', '<leader>ge', live_grep, { desc = '[S]earch by [G]rep' })

-- The line beneath this is called `modeline`. See `:help modeline`
-- vim: ts=2 sts=2 sw=2 et

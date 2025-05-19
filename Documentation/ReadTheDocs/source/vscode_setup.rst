VS Code + GitHub Quick-Start
==============================

Download and Install VS Code
---------------------------

1. Go to the official site: https://code.visualstudio.com.
2. Download the installer for **Windows / macOS / Linux**.
3. Run the installer (keep default options) and launch *Visual Studio Code*.

First-Launch Checklist
----------------------

* Allow VS Code to add itself to the system **PATH** when prompted.
* Sign in with your Microsoft or GitHub account (optional but recommended).
* Dismiss the introductory "Get Started" page once you’re ready.

Install the GitHub Extension Pack
---------------------------------

VS Code’s Git integration works out of the box, but the official extension
adds Pull-Request and Issue features.

1. Press ``Ctrl + Shift + X`` (*Extensions* side-bar).
2. Search for *GitHub Pull Requests and Issues* and click **Install**.
3. When prompted, **Sign in to GitHub** → Authorize in your browser.

(Optional) GitHub Copilot
~~~~~~~~~~~~~~~~~~~~~~~~~
If you have access to Copilot, install **GitHub Copilot** from the same
marketplace and sign in; otherwise ignore this step.

Clone a Repository from GitHub
-----------------------------

1. Press ``F1`` (or ``Ctrl + Shift + P``) → **Git: Clone**.
2. Paste the repository HTTPS/SSH URL, e.g. ``https://github.com/YourOrg/Project.git``.
3. Choose a local folder; VS Code reloads and opens the repo.

Editing, Committing, Pushing
----------------------------

* **Edit** files in the *Explorer* side-bar (``Ctrl + Shift + E``).
* **Save** often – VS Code auto-formats if you enable *Format on Save*.
* **Stage & Commit**
  1. Click the *Source Control* icon (``Ctrl + Shift + G``).
  2. Review changes, hit **+** to stage or **Commit** directly.
  3. Enter a commit message and press ``Ctrl + Enter``.
* **Push** to GitHub: click the … menu → **Push** (or use the blue status-bar
  cloud icon). The first push may prompt for branch name or to **Publish Branch**.

Creating a New File on GitHub (Browser)
---------------------------------------

1. Navigate to the repo on github.com.
2. Click **Add file → Create new file**.
3. Name the file (e.g. ``README.md``), paste content, commit.
4. Pull the change locally: ``Git: Pull`` in VS Code.

Syncing a Fork or Upstream Changes
----------------------------------

*Add the original repo as* **upstream**::

   git remote add upstream https://github.com/Original/Repo.git
   git fetch upstream
   git merge upstream/main          # or rebase

Then **Push** to your fork.

Handy Shortcuts
---------------

===================  =============================
Action               Shortcut (Win/Linux defaults)
===================  =============================
Command Palette      ``Ctrl + Shift + P``
File Explorer        ``Ctrl + Shift + E``
Source Control       ``Ctrl + Shift + G``
Open Terminal        ``Ctrl + ` ``
Search + Replace     ``Ctrl + Shift + F``
Run / Debug          ``F5``
===================  =============================

Troubleshooting
---------------

* **Failed to push – authentication error** → Ensure you completed the browser
  sign-in pop-up or set a *personal-access token* if using HTTPS.
* **“PR branch not found”** → Click **Fetch** first, then checkout.
* **Merge conflicts** → VS Code highlights conflict markers ««« === »»». Pick
  *Accept Current* / *Incoming* in the *Code Lens* above each block.

You’re ready—happy coding! :rocket:
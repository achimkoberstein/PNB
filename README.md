
# Installation

INSTALL file is outdated and for CBC package.

This package contains several subdirectories corresponding to COIN-OR
projects (www.coin-or.org). The AUTHORS, LICENSE and README files in 
each of the subdirectories give more information about these projects.

__PNB is tested and supported for Windows 10 x64 Pro and Visual Studio 2012 / 2019.__

## Prerequisites:
Windows 10 x64 Pro, Visual Studio 2012 or 2019, Git version >= 2.19.1.windows.1

## Get the source code and projects for Visual Studio
1. go to [https://github.com/pashtetGP/PNB/releases](https://github.com/pashtetGP/PNB/releases) and download the newest release (about 300 MB).
1. unpzip and rename the PNB-x.x.x.zip\PNB-x.x.x folder, for example, to C:\Users\\\<user>\Documents\PNB.
The folder structure for the aforementioned example would be so that Data folder is C:\Users\\\<user>\Documents\PNB\Data.

## Checkout the git branch from the server:
After completion of the steps in "Get the source code and projects for Visual Studio" run in command line:
1. `git lfs install` - to install the support of large file storage (some files in the repository are > 100 MB)
1. `cd C:\Users\<user>\Documents\CodingProjects\PNB` - cd to the PNB location
1. `git init` - start a new git repository
1. `git remote add origin https://github.com/pashtetGP/PNB.git` - add remote
1. `git fetch --all` - get info about remote branches
1. `git reset --hard origin/master` - checkous master branch from remote

You will end up on the master branch. To checkout another branch:

    git checkout -t origin/<branch>
  ## Useful git script
See scripts/
 
 ### git_upd current branch (keep local changes).bat
- will stash the local not commited changes
- get the newest commits from the GitHub server for the current branch
- reapply the local changes to keep them
The script will crash:
- in case of local commits not present on the remote branch
 
 ### git_update VS2019 branch (discard local changes).bat
- will __delete__ all the local not commited changes
- get the newest commits from the GitHub server for the VS2019 branch
- overwrite the local commit history with the remote one
 
  ## Branches
1. __master__ - default production branch, protected
1. __VS2019__ - postponed due to the low priority, transfer all projects to VS 2019 [done #7], use of COIN-OR and third party projects as git submodules

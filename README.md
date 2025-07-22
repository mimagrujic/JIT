# 🛠️ Jit - Simplified Git

Jit is a simplified version of Git written in C++. 

## 🚀 Features
  - Repository initialization
      - Creation of .jit directory with all its necessary components(`objects`, `refs/branches`, `head` and `index`)
  - Staging
      - Creating blob objects out of existing files in current working directory
      - Updating index file accordingly
  - Committing
      - Creating working repo's tree object and commit object
  - Branch operations
      - Creating a new branch
      - Switching between branches
      - Deleting a branch
      - Listing current/all branches
  - Resetting
      - Implementation of all 3 reset options: `soft`, `mixed` and `hard`
  - Accessing commit history
  - Object type/content check:
      - Implementation of `git cat-file -t/-p <hash>` function
## 📦 Setup instructions
   1. Clone the repository:
      ```
      git clone https://github.com/mimagrujic/JIT.git
      ```
   2. Open the project in IDE or code editor
## 💻 Usage / Command reference
| Command                                      | Description                                      |
|----------------------------------------------|--------------------------------------------------|
| `jit help`                                   | Prints documentation with available commands    |
| `jit init <path/.>`                          | Initializes a new repository                    |
| `jit status`                                 | Shows the current status of the working tree    |
| `jit add <file>` or `jit add .`              | Stages a file or all files in the directory     |
| `jit commit <message>`                       | Commits staged changes with a message           |
| `jit branch <name>`                          | Creates a new branch                            |
| `jit goto <name>`                            | Switches to an existing branch                  |
| `jit goto new <name>`                        | Creates and switches to a new branch            |
| `jit current`                                | Shows the currently active branch               |
| `jit branches`                               | Lists all branches                              |
| `jit delete -b <branch>`                     | Deletes the specified branch                    |
| `jit (soft/mixed/hard) <HEAD~n / hash>`      | Performs a reset to a previous n-th commit      |
| `jit log`                                    | Displays commit history                         |
| `jit cat-file -t <hash>`                     | Shows the type of the object                    |
| `jit cat-file -p <hash>`                     | Prints the content of the object                |

## 🚧 Limitations / Known Issues
- Does not support merging, conflict resolution, or rebase operations
- Does not support working with remote repositories
## ✅ Project Goals
The goals of this project were to:
  - Deepen my understanding of Git
  - Understand how Git works under the hood
## 🗺️ Roadmap / Next Steps 
  - Merge and rebase support
  - Reflog implementation
## 📬 Contact
If you have questions or feedback, feel free to reach out via [LinkedIn](https://www.linkedin.com/in/mima-grujic/) or email.
    

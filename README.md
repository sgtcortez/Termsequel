# Summary
- [Termsequel](#Termsequel)    
    - [What is](#What-is)
    - [Installation](#Installation)
- [Users](#Users)
    + [Features](#Features)
        + [Columns](#Columns)
        + [SELECT](#SELECT)
- [Developers](#Developers)

# Termsequel
Bringing SQL to our filesystem.

- ## What is

    Termsequel, is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.    
    I am using C++, because, it is modern, nice, portable, and, I wanna learn it.    
    I am trying to learn compilers too, because of that, I havent use yacc & lex, and, I am building my own(Makes more portable too).

- ## Installation

    To install **Termsequel** on your operating system, just run(with superuser privilegies):   
    ```bash
    make install
    ```

    To create the binary to be executed, just run:     
    ```bash
    make
    ```

# Users
Some advices to run **Termsequel**.   
The system is not thread safe, because, there is no need to this. Well, at least for now.

- ## Features

    Some features that are ready to be used. For now, just the SELECT command is available, but, I want to implement some DML commands.    
    I am working to be able to use: [**shell expansion variables**](https://www.gnu.org/software/bash/manual/html_node/Shell-Parameter-Expansion.html). 

    For now, you can not make joins, select from multiple tables, and, I am not sure if it will be implemented.     
    
    - ### Columns

        The columns, is what you can obtain(just obtain, because where is not supported yet) of information in a command.
    
        * NAME

            Returns the name of the file.

        + SIZE

            Returns the size in bytes of the file.   
            Size of an directory may not be correct. Because, to get the real size, we need to retrive all the files/subdirectories, and calculate all of them.

    - ### SELECT

        The same syntax as a SQL database.    
        **Termsequel SELECT syntax:** `SELECT COLUMN_LIST FROM "FILE"` 

        **Example:** *"SELECT"* syntax 
        ```sql
        SELECT NAME FROM "<FILE>"    
        -- <FILE> is a placeholder. File can be a regular file, and a directory
        ```

        **Example:** Selecting names from user documents directory.    
        ```sql
        SELECT NAME FROM "/home/user/documents"
        -- OR, with shell expansion variables    
        SELECT NAME FROM "$HOME/documents"
        ```




# Developers
There are sections for developers to be able to join this project.

<details>
<summary>
Git Guidelines
</summary>
Feature branch <strong>must</strong> be created from the most stable branch(usually main), and, when done, should make a Pull Request to the <strong>development</strong> branch.  
The commits <strong>must</strong> be <strong><italic>"squashed"</italic></strong>. The person who accepts the pull request, must, do a rebase to pick the single commit and put it on the target branch. 
</details>
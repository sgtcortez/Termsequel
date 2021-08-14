# Summary
- [Termsequel](#Termsequel)    
    - [What is](#What-is)
    - [Goals](#Goals)
        + [Personal](#Personal)
        + [Project](#Project)
    - [Installation](#Installation)
        + [Linux](#Linux)
        + [Windows](#Windows)
        + [Mac](#Mac)
- [Users](#Users)
    + [Features](#Features)
        + [Columns](#Columns)
        + [SELECT](#SELECT)
- [Developers](#Developers)

# Termsequel
Bringing SQL to our filesystem.      
![Termsequel](./.assets/Termsequel-logo.png)

Open Source Project hosted on [Github](https://github.com/sgtcortez/Termsequel)

- ## What is

    Termsequel, is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.  

- ## Goals

    What are my goals and **Termsequel** goals?

    + ### Personal

        For me, I always wanted to learn C++ and Compilers. Well, at least the concepts of both.  
        And, I always wanted to have some project that really helps me on my daily routines, and **Termsequel** helps me. Because, it really helps me, I decided to make it open source, and try to help someone else(This is the objective of an Open Source Project, right? :grimacing: ).  

        I have never learned the [find](https://www.man7.org/linux/man-pages/man1/find.1.html) command very well, and I believe that it has a lot of information and does a lot of things(but is very useful).  

        I am not building **Termsequel** to be a replacement of **find**, not this, but, an alternative tool that is more user friendly for easy and basic tasks, like, search for a *.exe* file in a directory.

    + ### Project

        **Termsequel** aims to help programmers, system administrators, hackers and even, non technical people who likes to learn new things.     
        **Termsequel** aims to be the most easy(this is why, SQL is chosen) to people. Plataform independent(work in progress), no external dependencies(yes, I could use the [Boost](https://www.boost.org/)) because, first of all, I want to learn, and, there is no need for an external library. This project is simple(yet).

- ## Installation

    For now, you will need to compile the source to be able to use **Termsequel**.   
    I recommend the [**clang**](https://clang.llvm.org/) compiler.       

    - ### Linux

        To install **Termsequel** on your operating system, just run(with superuser privilegies):   
        ```bash
        make install
        ```

        To create the binary to be executed, just run:     
        ```bash
        make
        ```

    - ### Windows

        The development is in progress.

    - ### Mac

        I do not have an MacOS. So, I do not know how I will make it portable.

# Users
Some advices to run **Termsequel**.   
The system is not thread safe, because, there is no need to this. Well, at least for now.    

You will need to provide the command in quotes or double quotes, otherwise, it will be different command line arguments.    
**Example:** Running Termsequel   
```bash
$ termsequel 'SELECT NAME FROM /HOME'
```

- ## Features

    Some features that are ready to be used. For now, just the SELECT command is available, but, I want to implement some DML commands.    

    For now, you can not make joins, select from multiple tables, and, I am not sure if it will be implemented.     
    
    - ### Columns

        The columns, is what you can obtain(just obtain, because where is not supported yet) of information in a command.
    
        * NAME

            Returns the name of the file.

        + SIZE

            Returns the size in bytes of the file.   
            Size of an directory may not be correct. Because, to get the real size, we need to retrive all the files/subdirectories, and calculate all of them.

        + OWNER 

            Returns the owner of the file.  

    - ### SELECT

        The same syntax as a SQL database.    
        **Termsequel SELECT syntax:** `SELECT COLUMN_LIST FROM FILE` 

        **Example:** *"SELECT"* syntax 
        ```sql
        SELECT NAME FROM <FILE>    
        -- <FILE> is a placeholder. File can be a regular file, and a directory. Can even be pseudo directories
        SELECT NAME FROM . -- Current directory
        SELECT NAME FROM ../ -- Parent directory
        ```

        **Example:** Selecting names from user documents directory.    
        ```sql
        SELECT NAME FROM /home/user/documents
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
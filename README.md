# Summary
- [Termsequel](#Termsequel)    
    - [What is](#What-is)    
- [Users](#Users)
    + [Features](#Features)
        + [Columns](#Columns)
        + [SELECT](#SELECT)
        + [WHERE](#WHERE)
- [Examples](#Examples)
    - [Getting name and size information](#Getting-name-and-size-information)

# Termsequel
Bringing SQL to our filesystem.      
![Termsequel](./.assets/Termsequel-logo.png)

Open Source Project hosted on [Github](https://github.com/sgtcortez/Termsequel)

- ## What is

    Termsequel, is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.  

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

            The name of the file.

        + SIZE

            The size in bytes of the file.

        + OWNER 

            The owner of the file.

        + LEVEL

            The depth level. Considering the from value as the relative path(if a directory).
            
        + FILE_TYPE

            The type of the file.

        + OWNER_PERMISSIONS

            Returns the permissions of the owner of the file.     
            The value, must be in octal mode!

        + GROUP_PERMISSIONS

            **Just for Linux**.    
            Returns the permissions of the group of the file.     
            The value, must be in octal mode!

        + OTHERS_PERMISSIONS

            **Just for Linux**.    
            Returns the permissions of other users.    
            The value, must be in octal mode!

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

    - ### WHERE

        Now, **Termsequel** supports **WHERE** filters!   
        You can write filters like:   
        ```sql
        SELECT NAME FROM . WHERE OWNER = myuser
        ```
        Even, you can group logical operators(for now, they are just left precedence!)  
        ```sql
        SELECT NAME FROM . WHERE OWNER = myuser AND SIZE > 10
        ```
        The logical operators are: **AND** and **OR**.    

        **Example:** Check if owner has execute permission in a file:    
        ```sql
        SELECT NAME, OWNER_PERMISSIONS FROM . WHERE OWNER_PERMISSIONS CONTAINS X
        -- Returns the files that the owner has the execute flag set.
        SELECT NAME, OWNER_PERMISSIONS FROM . WHERE OWNER_PERMISSIONS CONTAINS R AND OWNER_PERMISSIONS CONTAINS X   
        -- Returns the files that the owner has the read and execute flag set.
        ```

# Examples
This section shows some examples of **termsequel** use ...

- ## Getting name and size information

    The most basic information about a file, is it's name, and it's size. The example below, shows how get this information ...    
    **Example:**    
    ```sql
    SELECT NAME, SIZE FROM . 
    ```
    **Explanation:** The **"."** symbol means: "the current working directory".
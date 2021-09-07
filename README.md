# Summary

- [Termsequel](#Termsequel)    
- [Examples](#Examples)
    - [Basic](#Basic)
    - [Advanced](#Advanced)
- [Wiki](#Wiki)    
- [Stars](#Stars)

# Termsequel

Bringing SQL to our filesystem.      
[Termsequel](https://github.com/sgtcortez/Termsequel), is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.  

# Examples

This section shows some examples of **termsequel** use ...      
Some useful information before:       
```txt
 .                   Means the current working directory 
 ../                 Means the parent directory of the current working directory   
 ../..               Means the parent directory of the parent directory of the current working directory 
 ./<SOME_DIRECTORY>  Means the relative path of a directory 
 R                   Means read permission 
 W                   Means write permission 
 X                   Means execute permission 
 $<NAME>             Means a variable
```

**Note:** In the follow examples, we use the *SQL* format syntax for simplicity and pretty examples. When using **Termsequel**, you will need to provide the binary name, and the SQL instruction **must** be inside quotes!   
**Example:** Correct usage of **Termsequel**  
```shell
$ termsequel 'SQL_INSTRUCTION'
```

To see the columns that are available on your binary, type:   
```shell
$ termsequel -h
```

- ## Basic

    Some basic examples of **Termsequel**.

    **Example:** Getting the name and size of a file in the current working directory. 
    ```sql
    SELECT NAME, SIZE FROM .
    ```

    **Example:** Getting `.txt` files in the current working directory.   
    ```sql
    SELECT NAME FROM . WHERE NAME ENDS_WITH .txt
    ```

    **Example:** Getting files bigger than 1500 bytes.   
    ```sql
    SELECT NAME, SIZE FROM . WHERE SIZE > 1500
    ```

- ## Advanced

    Some advanced examples of **Termsequel**.

    **Example:** Getting files from parent directory whose owner has execution permission.   
    ```sql
    SELECT NAME, OWNER_PERMISSIONS FROM ../ WHERE OWNER_PERMISSIONS CONTAINS X
    ```

    **Example:** Getting files from user *home* directory that ends with `.pdf` and the owner has execution permission ...
    ```sql
    SELECT NAME, ABSOLUTE_PATH FROM $HOME WHERE NAME ENDS_WITH .pdf AND OWNER_PERMISSION CONTAINS X
    ```

# Wiki

The [wiki](https://github.com/sgtcortez/Termsequel/wiki) page contains the documentation of the termsequel.   
Additional information about compilers, CMake, operating systems can be found there ... 

# Stars

As an open source project, we need to mantain ourselfs motivated ... Of couse, sometimes, we lost the motivation, and this is a normal thing ...
I do not want money for this project, do something else with your money ... Buy a thing for you, your mummy etc ... 
One thing, that you can help, is to give a star ... With this, I know that I am helping someone around the world, and, this motivates me to continue ...

A special thanks to the people who give an [star](https://github.com/sgtcortez/Termsequel/stargazers) ... 

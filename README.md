# Summary
- [Termsequel](#Termsequel)    
- [Examples](#Examples)
    - [Basic](#Basic)
    - [Advanced](#Advanced)
    
# Termsequel
Bringing SQL to our filesystem.      
Termsequel, is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.  

# Examples
This section shows some examples of **termsequel** use ...

Some useful information before:   
| Item | Description | 
| :-:  | :-:|
| **.**    | Means the current working directory |
| **../**  | Means the parent directory of the current working directory |  
| **../..**| Means the parrent directory of the parent directory of the current working directory |
| **./<SOME_DIRECTORY>** | Means the relative path of a directory |
| **R** | Means read permission |
| **W** | Means write permission |
| **X** | Means execute permission |


**Note:** In the follow examples, we use the *SQL* format syntax for simplicity and pretty examples. When using **Termsequel**, you will need to provide the binary name, and the SQL instruction **must** be inside quotes!   
**Example:** Correct usage of **Termsequel**  
```shell
$ termsequel 'SQL_INSTRUCTION'
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
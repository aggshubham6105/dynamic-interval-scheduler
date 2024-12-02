# Dynamic Task Scheduler

This is a simple task scheduling application implemented in C++. It allows users to add, delete, and query tasks based on specific time intervals for the year 2025.

## Features

- Add tasks with specific start and end times.
- Delete existing tasks.
- Query the number of tasks scheduled at a specific time.
- Retrieve tasks scheduled within a certain time interval.
- Add recurring tasks at specific intervals.
- Get the earliest task scheduled.
- Display all scheduled tasks.

## Requirements

- C++11 or later
- A C++ compiler (e.g., g++, clang++)

## How to Run

1. **Clone the Repository** (if applicable):
   ```bash
   git clone <repository-url>
   cd <repository-folder>

2. **Compile the Code**:
   Use a terminal or command prompt and run:
   ```bash
   g++ -o project project.cpp

3. **Run the Program**:
   After compilation, run the program with:
   ```bash
   ./project

## User Input

Once the program is running, you will see a menu with several options:

1. **Add Task**: Enter the task's month, day, start hour, end hour, and a description. 
   - **Start and End Hour**: Input hours in 24-hour format (0-24). The end hour is exclusive; you must input it accordingly.
   
2. **Delete Task**: Enter the details of the task you wish to delete (month, day, start hour, end hour).

3. **Get Task Count at Time**: Enter a specific date and hour to get the number of tasks scheduled at that time.

4. **Get Tasks by Interval**: Input the month, day, start hour, and end hour to retrieve all tasks scheduled within that interval.

5. **Get Earliest Task Start Time**: This option will return the earliest scheduled task.

6. **Display All Tasks**: This will list all scheduled tasks in a tabular format.

7. **Add Recurring Task**: Enter the task’s month, day, start hour, end hour, frequency (in hours), and a description to create a recurring task.

9. **Exit**: Exit the application.

## Expected Output

- After each operation (add, delete, query), the program will provide feedback on the success of the action taken.
- For displaying tasks, it will show a list of tasks along with their scheduled dates and times.
- If you attempt to add a task that overlaps with an existing one, you'll be prompted to either merge the tasks or find the nearest available time interval.

## Example

### Adding a Task

```plaintext
Enter task details:
Month (1-12): 12
Day (1-31): 25
Start Time (Hour): 10
End Time (Hour): 12
Task Description: Christmas Meeting
```

### Querying Task Count

```plaintext
Enter time to get task count:
Month (1-12): 12
Day (1-31): 25
Hour: 11
Tasks at 25-12 11:00 = 1
```

### Displaying All Tasks

```plaintext
Scheduled Tasks:
Date      Time      Tasks
----------------------------------------
25-12    10-12    Christmas Meeting
```

### Adding Recurring Task

```plaintext
Enter recurring task details:
Month (1-12): 1
Day (1-31): 1
Start Time (Hour): 8
End Time (Hour): 9
Frequency (in hours): 24
Task Description: Daily Morning Briefing
```

### Querying Recurring Task

```plaintext
Enter time to get task count:
Month (1-12): 1
Day (1-31): 2
Hour: 8
Tasks at 2-1 08:00 = 1
```


#include <chrono>
#include <climits>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

// Custom hash function for pair<int, int>
struct pair_hash {
  template <class T1, class T2> size_t operator()(const pair<T1, T2> &p) const {
    auto hash1 = hash<T1>{}(p.first);
    auto hash2 = hash<T2>{}(p.second);
    return hash1 ^ (hash2 << 1); // Combine hashes
  }
};

// Function to convert hours since the start of the year into a date string
string getDateFromHours(int hourOfYear) {
  tm timeIn = {0, 0, 0, 1, 0, 2025 - 1900}; // January 1, 2025
  time_t startOfYear = mktime(&timeIn);

  time_t targetTime = startOfYear + hourOfYear * 3600;
  tm *resultTime = localtime(&targetTime);

  ostringstream oss;
  oss << put_time(resultTime, "%d-%m %H:%M"); // Format: DD-MM HH:MM
  return oss.str();
}

// Function to calculate total hours from the start of the year to a given date
int dateToHourOfYear(int year, int month, int day, int hour) {
  tm timeIn = {0, 0, hour, day, month - 1, year - 1900}; // year is set to 2025
  time_t timeTemp = mktime(&timeIn);

  tm startOfYear = {0, 0, 0, 1, 0, year - 1900}; // January 1st, 2025
  time_t startTimeTemp = mktime(&startOfYear);

  return static_cast<int>(difftime(timeTemp, startTimeTemp) /
                          3600); // Return hours difference
}

struct SegmentTreeNode {
  int taskCount; // Number of tasks happening at this exact interval
  int lazy;      // Lazy value for range updates

  SegmentTreeNode() : taskCount(0), lazy(0) {}
};

class SegmentTree {
private:
  vector<SegmentTreeNode> tree;
  int size;

  void propagate(int node, int start, int end) {
    if (tree[node].lazy != 0) {
      tree[node].taskCount += tree[node].lazy * (end - start + 1);
      if (start != end) { // Not a leaf node
        tree[node * 2 + 1].lazy += tree[node].lazy;
        tree[node * 2 + 2].lazy += tree[node].lazy;
      }
      tree[node].lazy = 0; // Clear the lazy value
    }
  }

  void update(int node, int start, int end, int l, int r, int count) {
    propagate(node, start, end); // Ensure any pending updates are applied

    if (start > end || start > r || end < l)
      return;

    if (start >= l && end <= r) {
      tree[node].lazy += count;    // Set lazy value
      propagate(node, start, end); // Apply the update immediately
      return;
    }

    int mid = (start + end) / 2;
    update(node * 2 + 1, start, mid, l, r, count);
    update(node * 2 + 2, mid + 1, end, l, r, count);
    tree[node].taskCount =
        tree[node * 2 + 1].taskCount + tree[node * 2 + 2].taskCount;
  }

  int query(int node, int start, int end, int l, int r) {
    propagate(node, start, end); // Ensure any pending updates are applied

    if (start > end || start > r || end < l)
      return 0;

    if (start >= l && end <= r) {
      return tree[node].taskCount;
    }

    int mid = (start + end) / 2;
    int leftQuery = query(node * 2 + 1, start, mid, l, r);
    int rightQuery = query(node * 2 + 2, mid + 1, end, l, r);
    return leftQuery + rightQuery;
  }

public:
  SegmentTree(int n) {
    size = n;
    tree.resize(4 * n);
  }

  void addTask(int start, int end, int count) {
    update(0, 0, size - 1, start, end, count);
  }

  int queryTasksInRange(int start, int end) {
    return query(0, 0, size - 1, start, end);
  }

  int queryTasksAtTime(int time) { return query(0, 0, size - 1, time, time); }

  int getSize() { return size; }
};

class TaskScheduler {
private:
  SegmentTree segmentTree;
  unordered_map<pair<int, int>, vector<string>, pair_hash>
      taskMap;                    // Store tasks by interval
  set<pair<int, int>> startTimes; // Stores the start times of all tasks

  pair<int, int> findNearestAvailableInterval(int start, int end, int offset) {
    int taskIntervalLength = end - start;
    int mid = start; // Start searching from the midpoint
    int leftRange = max(0, start - offset);
    int rightRange = min(start + offset, segmentTree.getSize() - 1);

    for (int off = 0; off <= offset; ++off) {
      // Check left side
      if (mid - off >= leftRange) {
        if (segmentTree.queryTasksInRange(
                mid - off, mid - off + taskIntervalLength) == 0) {
          return {mid - off, mid - off + taskIntervalLength};
        }
      }

      // Check right side
      if (mid + off <= rightRange) {
        if (segmentTree.queryTasksInRange(
                mid + off, mid + off + taskIntervalLength) == 0) {
          return {mid + off, mid + off + taskIntervalLength};
        }
      }
    }

    return {-1, -1}; // No available range within offset
  }

public:
  TaskScheduler(int n) : segmentTree(n) {}

  void addTask(int year, int month, int day, int startHour, int endHour,
               const string &taskDescription) {
    int start = dateToHourOfYear(year, month, day, startHour);
    int end = dateToHourOfYear(year, month, day, endHour);
    pair<int, int> interval = {start, end};
    int existingTaskCount = segmentTree.queryTasksInRange(start, end);

    if (existingTaskCount > 0) {
      cout << "Tasks already exist from time " << startHour << "-"
           << endHour + 1 << "\n";
      cout << "Please choose an option:\n";
      cout << "1. Merge the task with existing interval.\n";
      cout << "2. Find the nearest available interval within an offset.\n";
      int choice;
      cin >> choice;

      if (choice == 1) {
        taskMap[interval].push_back(taskDescription);
        startTimes.insert(interval);
        segmentTree.addTask(start, end, 1); // Update entire range
        cout << "Task merged successfully.\n";
      } else if (choice == 2) {
        int offset;
        cout << "Enter the offset value: ";
        cin >> offset;

        pair<int, int> newInterval =
            findNearestAvailableInterval(start, end, offset);
        if (newInterval.first != -1) {
          cout << "Nearest available interval is ["
               << getDateFromHours(newInterval.first) << ", "
               << getDateFromHours(newInterval.second + 1) << "].\n";
          addTask(year, month, day, newInterval.first, newInterval.second,
                  taskDescription); // Add the task
        } else {
          cout << "No available interval within the specified offset.\n";
        }
      }
    } else {
      taskMap[interval].push_back(taskDescription);
      startTimes.insert(interval);
      segmentTree.addTask(start, end, 1); // Update entire range
      cout << "Task added successfully.\n";
    }
  }

  void deleteTask(int year, int month, int day, int startHour, int endHour) {
    int start = dateToHourOfYear(year, month, day, startHour);
    int end = dateToHourOfYear(year, month, day, endHour);
    pair<int, int> interval = {start, end};

    if (taskMap.find(interval) != taskMap.end()) {
      taskMap.erase(interval);
      startTimes.erase(interval);
      segmentTree.addTask(start, end, -1); // Decrease task count for range
      cout << "Task deleted successfully.\n";
    } else {
      cout << "No task found in the interval [" << start << ", " << end
           << "].\n";
    }
  }

  int getTaskCountAtTime(int year, int month, int day, int hour) {
    int time = dateToHourOfYear(year, month, day, hour);
    return segmentTree.queryTasksAtTime(time);
  }

  vector<string> getTasksByInterval(int year, int month, int day, int startHour,
                                    int endHour) {
    int start = dateToHourOfYear(year, month, day, startHour);
    int end = dateToHourOfYear(year, month, day, endHour);
    pair<int, int> interval = {start, end};

    if (taskMap.find(interval) != taskMap.end()) {
      return taskMap[interval];
    }
    return {}; // Return empty vector if no tasks scheduled
  }

  pair<int, int> getEarliestTask() {
    if (!startTimes.empty()) {
      auto earliest = *startTimes.begin();      // Get the earliest interval
      return {earliest.first, earliest.second}; // Return start, end
    }
    return {-1, -1}; // No tasks present
  }

  void displayAllTasks() {
    cout << "\nScheduled Tasks:\n";
    cout << left << setw(10) << "Date" << setw(10) << "Time"
         << "Tasks\n";
    cout << string(40, '-') << endl; // Table header

    for (auto interval : startTimes) {
      const auto &tasks = taskMap[interval];
      string startDate = getDateFromHours(interval.first).substr(0, 5);
      string startHour = getDateFromHours(interval.first).substr(6, 2);
      string endHour = getDateFromHours(interval.second + 1).substr(6, 2);

      // Join tasks into a single string
      string taskList;
      for (const auto &task : tasks) {
        if (!taskList.empty()) {
          taskList += ", "; // Add comma between tasks
        }
        taskList += task;
      }

      cout << left << setw(10) << startDate << setw(10)
           << startHour + "-" + endHour << taskList << endl;
    }
  }
};

int main() {
  int n = 365 * 24; // Maximum hours in 2025 (365 days * 24 hours)
  TaskScheduler scheduler(n);

  while (true) {
    cout << "\nTask Scheduler Menu:\n";
    cout << "1. Add Task\n";
    cout << "2. Delete Task\n";
    cout << "3. Get Task Count at Time\n";
    cout << "4. Get Tasks by Interval\n";
    cout << "5. Get Earliest Task Start Time\n";
    cout << "6. Display All Tasks\n"; // New option
    cout << "7. Exit\n";
    cout << "Choose an option: ";

    int choice;
    cin >> choice;

    if (choice == 1) {
      int month, day, startHour, endHour;
      string description;
      cout << "Enter task details:\n";
      cout << "Month (1-12): ";
      cin >> month;
      cout << "Day (1-31): ";
      cin >> day;
      cout << "Start Time (Hour): ";
      cin >> startHour;
      cout << "End Time (Hour): ";
      cin >> endHour;
      endHour--;    // Adjust end hour
      cin.ignore(); // To consume the newline character
      cout << "Task Description: ";
      getline(cin, description);
      scheduler.addTask(2025, month, day, startHour, endHour, description);
    } else if (choice == 2) {
      int month, day, startHour, endHour;
      cout << "Enter task details to delete:\n";
      cout << "Month (1-12): ";
      cin >> month;
      cout << "Day (1-31): ";
      cin >> day;
      cout << "Start Time (Hour): ";
      cin >> startHour;
      cout << "End Time (Hour): ";
      cin >> endHour;
      endHour--; // Adjust end hour
      scheduler.deleteTask(2025, month, day, startHour, endHour);
    } else if (choice == 3) {
      int month, day, hour;
      cout << "Enter time to get task count: ";
      cout << "Month (1-12): ";
      cin >> month;
      cout << "Day (1-31): ";
      cin >> day;
      cout << "Hour: ";
      cin >> hour;
      cout << "Tasks at " << day << "-" << month << " " << hour
           << ":00 = " << scheduler.getTaskCountAtTime(2025, month, day, hour)
           << endl;
    } else if (choice == 4) {
      int month, day, startHour, endHour;
      cout << "Enter month (1-12) and day to get tasks: \n";
      cout << "Month (1-12): ";
      cin >> month;
      cout << "Day (1-31): ";
      cin >> day;
      cout << "Start Time (Hour): ";
      cin >> startHour;
      cout << "End Time (Hour): ";
      cin >> endHour;
      endHour--; // Adjust end hour
      vector<string> tasks =
          scheduler.getTasksByInterval(2025, month, day, startHour, endHour);
      cout << "Tasks scheduled: ";
      if (tasks.empty()) {
        cout << "No tasks scheduled.";
      } else {
        for (const auto &task : tasks) {
          cout << task << " ";
        }
      }
      cout << endl;
    } else if (choice == 5) {
      auto earliest = scheduler.getEarliestTask();
      if (earliest.first != -1) {
        cout << "Earliest task starts at: " << getDateFromHours(earliest.first)
             << " and ends at: " << getDateFromHours(earliest.second + 1)
             << endl;
      } else {
        cout << "No tasks scheduled." << endl;
      }
    } else if (choice == 6) {
      scheduler.displayAllTasks(); // Call the new display function
    } else if (choice == 7) {
      break; // Exit the loop
    } else {
      cout << "Invalid option. Please try again." << endl;
    }
  }

  return 0;
}

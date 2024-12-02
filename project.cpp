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
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2> &p) const {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ (hash2 << 1); // Combine hashes
    }
};

// Function to convert hours since the start of the year into a date string
string getDateFromHours(int hourOfYear) {
    tm timeIn = {};
    timeIn.tm_year = 2025 - 1900; // Year 2025
    timeIn.tm_mon = 0;            // January
    timeIn.tm_mday = 1;           // Day 1
    timeIn.tm_hour = 0;           // Midnight
    timeIn.tm_min = 0;
    timeIn.tm_sec = 0;
    timeIn.tm_isdst = -1;         // Let mktime determine DST

    time_t startOfYear = mktime(&timeIn);
    if (startOfYear == -1) {
        return "Invalid Date";
    }

    time_t targetTime = startOfYear + hourOfYear * 3600;
    tm *resultTime = localtime(&targetTime);
    if (!resultTime) {
        return "Invalid Date";
    }

    ostringstream oss;
    oss << put_time(resultTime, "%d-%m %H:%M");
    return oss.str();
}

// Function to calculate total hours from the start of the year to a given date
int dateToHourOfYear(int year, int month, int day, int hour) {
    tm timeIn = {};
    timeIn.tm_sec = 0;
    timeIn.tm_min = 0;
    timeIn.tm_hour = hour;
    timeIn.tm_mday = day;
    timeIn.tm_mon = month - 1;
    timeIn.tm_year = year - 1900;
    timeIn.tm_isdst = -1; // Let mktime determine DST

    time_t timeTemp = mktime(&timeIn);
    if (timeTemp == -1) {
        return -1;
    }

    tm startOfYear = {};
    startOfYear.tm_sec = 0;
    startOfYear.tm_min = 0;
    startOfYear.tm_hour = 0;
    startOfYear.tm_mday = 1;
    startOfYear.tm_mon = 0;
    startOfYear.tm_year = year - 1900;
    startOfYear.tm_isdst = -1;

    time_t startTimeTemp = mktime(&startOfYear);
    if (startTimeTemp == -1) {
        return -1;
    }

    double diff = difftime(timeTemp, startTimeTemp);
    if (diff < 0) {
        return -1;
    }

    return static_cast<int>(diff / 3600);
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
        tree[node].taskCount = tree[node * 2 + 1].taskCount + tree[node * 2 + 2].taskCount;
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
    unordered_map<pair<int, int>, vector<string>, pair_hash> taskMap; // Store tasks by interval
    set<pair<int, int>> startTimes; // Stores the start times of all tasks

    // Helper function to find the nearest available interval within an offset
    pair<int, int> findNearestAvailableInterval(int start, int end, int offset) {
        int taskIntervalLength = end - start;
        int leftRange = max(0, start - offset);
        int rightRange = min(start + offset, segmentTree.getSize() - 1);

        for (int off = 0; off <= offset; ++off) {
            // Check left side
            if (start - off >= leftRange && (start - off + taskIntervalLength) < segmentTree.getSize()) {
                if (segmentTree.queryTasksInRange(start - off, start - off + taskIntervalLength - 1) == 0) {
                    return {start - off, start - off + taskIntervalLength};
                }
            }

            // Check right side
            if (start + off + taskIntervalLength <= rightRange) {
                if (segmentTree.queryTasksInRange(start + off, start + off + taskIntervalLength - 1) == 0) {
                    return {start + off, start + off + taskIntervalLength};
                }
            }
        }

        return {-1, -1}; // No available range within offset
    }

    // Helper function to add a task using total hours
    void addTaskByTotalHours(int startHourOfYear, int endHourOfYear, const string &taskDescription) {
        // Validate hours
        if (startHourOfYear < 0 || endHourOfYear > segmentTree.getSize() || startHourOfYear >= endHourOfYear) {
            cout << "Invalid interval for the task.\n";
            return;
        }

        pair<int, int> interval = {startHourOfYear, endHourOfYear};
        int existingTaskCount = segmentTree.queryTasksInRange(startHourOfYear, endHourOfYear - 1);

        if (existingTaskCount > 0) {
            cout << "Tasks already exist from time " << getDateFromHours(startHourOfYear)
                 << " - " << getDateFromHours(endHourOfYear) << "\n";
            // Optionally, you can handle further conflict resolution here
        } else {
            taskMap[interval].push_back(taskDescription);
            startTimes.insert(interval);
            segmentTree.addTask(startHourOfYear, endHourOfYear - 1, 1); // Update entire range
            cout << "Task added successfully.\n";
        }
    }

public:
    TaskScheduler(int n) : segmentTree(n) {}

    // Single addTask function
    void addTask(int year, int month, int day, int startHour, int endHour, const string &taskDescription) {
        // Convert to total hours
        int start = dateToHourOfYear(year, month, day, startHour);
        int end = dateToHourOfYear(year, month, day, endHour);

        if (start == -1 || end == -1 || start >= end) {
            cout << "Invalid date or time input.\n";
            return;
        }

        pair<int, int> interval = {start, end};
        int existingTaskCount = segmentTree.queryTasksInRange(start, end - 1);

        if (existingTaskCount > 0) {
            cout << "Tasks already exist from time " << startHour << "-" << endHour << "\n";
            cout << "Please choose an option:\n";
            cout << "1. Merge the task with existing interval.\n";
            cout << "2. Find the nearest available interval within an offset.\n";
            int choice;
            cin >> choice;

            if (choice == 1) {
                taskMap[interval].push_back(taskDescription);
                startTimes.insert(interval);
                segmentTree.addTask(start, end - 1, 1); // Update entire range
                cout << "Task merged successfully.\n";
            } else if (choice == 2) {
                int offset;
                cout << "Enter the offset value: ";
                cin >> offset;

                pair<int, int> newInterval = findNearestAvailableInterval(start, end, offset);
                if (newInterval.first != -1) {
                    cout << "Nearest available interval is ["
                         << getDateFromHours(newInterval.first) << ", "
                         << getDateFromHours(newInterval.second) << "].\n";
                    // Add the task using the helper function
                    addTaskByTotalHours(newInterval.first, newInterval.second, taskDescription);
                } else {
                    cout << "No available interval within the specified offset.\n";
                }
            } else {
                cout << "Invalid choice. Task not added.\n";
            }
        } else {
            taskMap[interval].push_back(taskDescription);
            startTimes.insert(interval);
            segmentTree.addTask(start, end - 1, 1); // Update entire range
            cout << "Task added successfully.\n";
        }
    }

    void deleteTask(int year, int month, int day, int startHour, int endHour) {
        int start = dateToHourOfYear(year, month, day, startHour);
        int end = dateToHourOfYear(year, month, day, endHour);

        if (start == -1 || end == -1 || start >= end) {
            cout << "Invalid date or time input.\n";
            return;
        }

        pair<int, int> interval = {start, end};

        if (taskMap.find(interval) != taskMap.end()) {
            taskMap.erase(interval);
            startTimes.erase(interval);
            segmentTree.addTask(start, end - 1, -1); // Decrease task count for range
            cout << "Task deleted successfully.\n";
        } else {
            cout << "No task found in the interval [" << getDateFromHours(start) << ", "
                 << getDateFromHours(end) << "].\n";
        }
    }

    int getTaskCountAtTime(int year, int month, int day, int hour) {
        int time = dateToHourOfYear(year, month, day, hour);
        if (time == -1) {
            cout << "Invalid date or time input.\n";
            return 0;
        }
        return segmentTree.queryTasksAtTime(time);
    }

    vector<string> getTasksByInterval(int year, int month, int day, int startHour, int endHour) {
        int start = dateToHourOfYear(year, month, day, startHour);
        int end = dateToHourOfYear(year, month, day, endHour);

        if (start == -1 || end == -1 || start >= end) {
            cout << "Invalid date or time input.\n";
            return {};
        }

        pair<int, int> interval = {start, end};

        if (taskMap.find(interval) != taskMap.end()) {
            return taskMap[interval];
        }
        return {}; // Return empty vector if no tasks scheduled
    }

    pair<int, int> getEarliestTask() {
        if (!startTimes.empty()) {
            auto earliest = *startTimes.begin(); // Get the earliest interval
            return {earliest.first, earliest.second}; // Return start, end
        }
        return {-1, -1}; // No tasks present
    }

    void displayAllTasks() {
        cout << "\nScheduled Tasks:\n";
        cout << left << setw(10) << "Date" << setw(12) << "Time" << "Tasks\n";
        cout << string(40, '-') << endl; // Table header

        for (auto interval : startTimes) {
            const auto &tasks = taskMap[interval];
            string startDate = getDateFromHours(interval.first).substr(0, 5);
            string startTime = getDateFromHours(interval.first).substr(6, 5);
            string endTime = getDateFromHours(interval.second).substr(6, 5);

            // Join tasks into a single string
            string taskList;
            for (const auto &task : tasks) {
                if (!taskList.empty()) {
                    taskList += ", "; // Add comma between tasks
                }
                taskList += task;
            }

            cout << left << setw(10) << startDate << setw(12)
                 << (startTime + "-" + endTime) << taskList << endl;
        }
    }

    // Function to add recurring tasks
    void addRecurringTask(int year, int month, int day, int startHour, int endHour, const string &taskDescription, int frequency) {
        // frequency is the number of hours between task repeats
        int start = dateToHourOfYear(year, month, day, startHour);
        int end = dateToHourOfYear(year, month, day, endHour);

        if (start == -1 || end == -1 || start >= end) {
            cout << "Invalid date or time input.\n";
            return;
        }

        // Add the task at the initial start time
        addTaskByTotalHours(start, end, taskDescription);

        // Add recurring tasks
        int nextStart = start + frequency;
        int nextEnd = end + frequency;
        while (nextStart < segmentTree.getSize()) {
            addTaskByTotalHours(nextStart, nextEnd, taskDescription);
            nextStart += frequency;
            nextEnd += frequency;
        }

        cout << "Recurring task added successfully.\n";
    }
};

// Main loop to interact with the task scheduler
int main() {
    TaskScheduler scheduler(365 * 24); // For example, 365 days with 24 hours per day

    while (true) {
        cout << "\n----------------------------------------\n";
        cout << "Task Scheduler Menu:\n";
        cout << "1. Add Task\n";
        cout << "2. Delete Task\n";
        cout << "3. Query Task Count at Time\n";
        cout << "4. Query Tasks by Interval\n";
        cout << "5. Display All Tasks\n";
        cout << "6. Add Recurring Task\n";
        cout << "7. Show Next Earliest Task\n";
        cout << "8. Exit\n";
        cout << "----------------------------------------\n";
        cout << "Enter your choice (1-8): ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            int year, month, day, startHour, endHour;
            string taskDescription;
            cout << "Enter year, month, day (YYYY MM DD): ";
            cin >> year >> month >> day;
            cout << "Enter start hour and end hour (0-23): ";
            cin >> startHour >> endHour;
            cout << "Enter task description: ";
            cin.ignore(); // To ignore leftover newline character
            getline(cin, taskDescription);

            scheduler.addTask(year, month, day, startHour, endHour, taskDescription);
            cout << "Task added successfully!\n";
        } else if (choice == 2) {
            int year, month, day, startHour, endHour;
            cout << "Enter year, month, day (YYYY MM DD): ";
            cin >> year >> month >> day;
            cout << "Enter start hour and end hour (0-23): ";
            cin >> startHour >> endHour;

            scheduler.deleteTask(year, month, day, startHour, endHour);
            cout << "Task deleted successfully!\n";
        } else if (choice == 3) {
            int year, month, day, hour;
            cout << "Enter year, month, day, hour (YYYY MM DD HH): ";
            cin >> year >> month >> day >> hour;
            cout << "Task count at the given time: " 
                 << scheduler.getTaskCountAtTime(year, month, day, hour) << endl;
        } else if (choice == 4) {
            int year, month, day, startHour, endHour;
            cout << "Enter year, month, day, start hour, end hour (YYYY MM DD HH HH): ";
            cin >> year >> month >> day >> startHour >> endHour;

            auto tasks = scheduler.getTasksByInterval(year, month, day, startHour, endHour);
            cout << "Tasks in the given interval:\n";
            for (const auto &task : tasks) {
                cout << task << endl;
            }
        } else if (choice == 5) {
            scheduler.displayAllTasks();
        } else if (choice == 6) {
            int year, month, day, startHour, endHour, frequency;
            string taskDescription;
            cout << "Enter year, month, day (YYYY MM DD): ";
            cin >> year >> month >> day;
            cout << "Enter start hour and end hour (0-23): ";
            cin >> startHour >> endHour;
            cout << "Enter frequency in hours: ";
            cin >> frequency;
            cout << "Enter task description: ";
            cin.ignore(); // To ignore leftover newline character
            getline(cin, taskDescription);

            scheduler.addRecurringTask(year, month, day, startHour, endHour, taskDescription, frequency);
            cout << "Recurring task added successfully!\n";
        } else if (choice == 7) {
            auto earliest = scheduler.getEarliestTask();
            if (earliest.first != -1) {
                cout << "Earliest task starts at: " << getDateFromHours(earliest.first)
                     << " and ends at: " << getDateFromHours(earliest.second) << endl;
            } else {
                cout << "No tasks scheduled." << endl;
            }
        } else if (choice == 8) {
            cout << "Exiting the Task Scheduler. Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice. Please enter a number between 1 and 8.\n";
        }
    }

    return 0;
}

#include <iostream>
#include <deque>
#include <string>
using namespace std;

#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define DIM   "\033[2m"

#define RED   "\033[31m"
#define GRN   "\033[32m"
#define YLW   "\033[33m"
#define CYN   "\033[36m"

#define BRED  "\033[91m"
#define BGRN  "\033[92m"
#define BYLW  "\033[93m"
#define BBLU  "\033[94m"
#define BCYN  "\033[96m"
#define BWHT  "\033[97m"
#define BBLK  "\033[90m"

struct Job {
    int id;
    string name;
    int pages;
    string priority;
    bool paused;
};

deque<Job> q;
int nextId = 1;

void clearScreen() {
    cout << "\033[2J\033[H";
}

void printHeader() {
    cout << "\n";
    cout << BOLD << BCYN << "  ██████  ██████   ██████   ██████  ██      ███████ ██████  " << RST << "\n";
    cout << BOLD << CYN  << "  ██   ██ ██   ██ ██    ██ ██    ██ ██      ██      ██   ██ " << RST << "\n";
    cout << BOLD << BCYN << "  ███████ ██████  ██    ██ ██    ██ ██      █████   ██████  " << RST << "\n";
    cout << BOLD << CYN  << "  ██      ██   ██ ██    ██ ██    ██ ██      ██      ██   ██ " << RST << "\n";
    cout << BOLD << BCYN << "  ██      ██   ██  ██████   ██████  ███████ ███████ ██   ██ " << RST << "\n";
    cout << BBLK << "\n                   print spooler  //  deque edition\n" << RST;
    cout << "\n";
}

void addJob(string name, int pages, string priority) {
    Job j;
    j.id = nextId;
    j.name = name;
    j.pages = pages;
    j.priority = priority;
    j.paused = false;
    nextId++;

    if (priority == "urgent") {
        q.push_front(j);
        cout << "\n  " << BGRN << "v " << RST << "Urgent job added to front: " << name << "\n";
    } else {
        q.push_back(j);
        cout << "\n  " << BGRN << "v " << RST << "Normal job added to rear: " << name << "\n";
    }
}

void processJob() {
    if (q.empty()) {
        cout << "\n  " << BRED << "x " << RST << "No jobs in queue!\n";
        return;
    }
    if (q.front().paused) {
        cout << "\n  " << BRED << "x " << RST << "Front job is paused. Resume it first.\n";
        return;
    }
    cout << "\n  " << BGRN << "v " << RST << "Processing: " << q.front().name << "\n";
    q.pop_front();
}

void cancelJob(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            cout << "\n  " << BGRN << "v " << RST << "Cancelled job: " << q[i].name << "\n";
            q.erase(q.begin() + i);
            return;
        }
    }
    cout << "\n  " << BRED << "x " << RST << "Job ID not found.\n";
}

void displayQueue() {
    cout << "\n";
    if (q.empty()) {
        cout << "  " << BBLK << "Queue is empty.\n" << RST;
        return;
    }

    int totalPages = 0;
    for (int i = 0; i < q.size(); i++) {
        totalPages += q[i].pages;
    }

    cout << BOLD << BWHT << "  Queue (" << q.size() << " jobs, " << totalPages << " pages total)\n\n" << RST;

    for (int i = 0; i < q.size(); i++) {
        Job j = q[i];

        if (i == 0)
            cout << BGRN << "  > " << RST;
        else
            cout << BBLK << "  " << (i + 1) << " " << RST;

        if (j.priority == "urgent")
            cout << BRED << BOLD << j.name << RST;
        else
            cout << BWHT << j.name << RST;

        cout << BBLK << "  |  " << RST;
        cout << BBLK << j.pages << " pages  |  " << RST;

        if (j.priority == "urgent")
            cout << BRED << "URGENT" << RST;
        else
            cout << BBLU << "normal" << RST;

        cout << BBLK << "  |  #" << j.id << RST;

        if (j.paused)
            cout << BYLW << "  [paused]" << RST;
        else if (i == 0)
            cout << BGRN << "  [printing]" << RST;

        cout << "\n";
    }
}

void countPages() {
    int total = 0;
    for (int i = 0; i < q.size(); i++) {
        total += q[i].pages;
    }
    cout << "\n  " << BWHT << "Total pages pending: " << total << RST << "\n";
}

void togglePause(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            q[i].paused = !q[i].paused;
            if (q[i].paused)
                cout << "\n  " << BYLW << "~ " << RST << "Job #" << id << " paused.\n";
            else
                cout << "\n  " << BGRN << "v " << RST << "Job #" << id << " resumed.\n";
            return;
        }
    }
    cout << "\n  " << BRED << "x " << RST << "Job ID not found.\n";
}

void promoteJob(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            Job j = q[i];
            q.erase(q.begin() + i);
            q.push_front(j);
            cout << "\n  " << BGRN << "v " << RST << "Job #" << id << " promoted to front.\n";
            return;
        }
    }
    cout << "\n  " << BRED << "x " << RST << "Job ID not found.\n";
}

void printMenu() {
    cout << "\n";
    cout << BBLK << "  --------------------------------------------------------\n" << RST;
    cout << "  " << BCYN  << "[a]" << RST << " add job    "
         << BGRN  << "[p]" << RST << " process    "
         << BRED  << "[c]" << RST << " cancel\n";
    cout << "  " << BYLW  << "[h]" << RST << " pause/res  "
         << BBLU  << "[m]" << RST << " promote    "
         << BWHT  << "[q]" << RST << " quit\n";
    cout << "\n  > ";
}

int main() {
    addJob("report.pdf", 12, "normal");
    addJob("ticket.pdf", 1, "urgent");
    addJob("notes.docx", 5, "normal");

    string choice;

    while (true) {
        clearScreen();
        printHeader();
        displayQueue();
        printMenu();

        cin >> choice;
        cin.ignore();

        if (choice == "q") {
            clearScreen();
            cout << "\n  " << BBLK << "Spooler shut down.\n\n" << RST;
            break;
        }

        else if (choice == "a") {
            string name, priority;
            int pages;
            cout << "\n  Document name: ";
            getline(cin, name);
            cout << "  Pages: ";
            cin >> pages;
            cin.ignore();
            cout << "  Priority (normal/urgent): ";
            getline(cin, priority);
            addJob(name, pages, priority);
        }

        else if (choice == "p") {
            processJob();
        }

        else if (choice == "c") {
            int id;
            cout << "\n  Enter job ID to cancel: ";
            cin >> id;
            cin.ignore();
            cancelJob(id);
        }

        else if (choice == "h") {
            int id;
            cout << "\n  Enter job ID to pause/resume: ";
            cin >> id;
            cin.ignore();
            togglePause(id);
        }

        else if (choice == "m") {
            int id;
            cout << "\n  Enter job ID to promote: ";
            cin >> id;
            cin.ignore();
            promoteJob(id);
        }

        else {
            cout << "\n  " << BRED << "x " << RST << "Invalid choice.\n";
        }

        cout << "\n  " << BBLK << "[enter to continue]" << RST;
        string t;
        getline(cin, t);
    }

    return 0;
}

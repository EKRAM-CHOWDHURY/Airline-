#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#endif

#endif
namespace color {
    constexpr const char* reset = "\x1b[0m";
    constexpr const char* bold  = "\x1b[1m";
    constexpr const char* dim   = "\x1b[2m";
    constexpr const char* red   = "\x1b[31m";
    constexpr const char* green = "\x1b[32m";
    constexpr const char* yellow= "\x1b[33m";
    constexpr const char* blue  = "\x1b[34m";
    constexpr const char* magenta="\x1b[35m";
    constexpr const char* cyan  = "\x1b[36m";
    constexpr const char* gray  = "\x1b[90m";
}

using namespace std;

int menu_ui_width = 66;

int read_int(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        int x;
        if (cin >> x) {
            if (x >= lo && x <= hi) return x;
            cout << "Please enter a number between " << lo << " and " << hi << ".\n";
        } else {
            // recover from non-numeric input
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Numbers only.\n";
        }
    }
}

bool ask_yes_no(const std::string& prompt, bool default_answer = false);
int calculate_ticket_price(const std::string& seat_class,
                           const std::string& food_menu);

#ifdef _WIN32
bool enable_vt_mode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    return SetConsoleMode(hOut, mode);
}
#endif

void clear_screen() {
#ifdef _WIN32
    static bool vt_ok = enable_vt_mode();
    if (vt_ok) {
        std::cout << "\x1b[2J\x1b[H";
    } else {
        std::system("cls");  // fallback if VT mode can't be enabled
    }
#else
    std::cout << "\x1b[2J\x1b[H";
#endif
    std::cout.flush();
}

//To store the total number of seats in the flight
int total_number_of_seats=100;
//To store if seat is booked or not, seat booked = -1, seat unbooked = 0
int seats[100] = {0};	

//To store the number of seats booked till now
int reserve_seats = 1000;

//To store the number of cancelled tickets booked till now
int cancel_tickets = 0;

// ---------- UI helper: yes/no prompt ----------
bool ask_yes_no(const std::string& prompt, bool default_answer) {
    // Show [Y/n] or [y/N] depending on default
    std::cout << prompt << ' '
              << (default_answer ? "[Y/n]: " : "[y/N]: ");

    while (true) {
        std::cin >> std::ws;  // eat whitespace/newlines
        int ch = std::cin.peek();

        // ENTER = take default
        if (ch == '\n') {
            std::cin.get();
            return default_answer;
        }

        char c;
        if (std::cin.get(c)) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (c == 'y') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return true;  }
            if (c == 'n') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return false; }

            // also accept "yes"/"no"
            std::string rest;
            std::getline(std::cin, rest);
            if (c == 'y' && rest == "es") return true;   // "yes"
            if (c == 'n' && rest == "o")  return false;  // "no"
        }

        std::cout << "Please enter y or n: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

class Flight{
	public : 
	   Flight()
	   {
		  start = NULL;
	   }
	   void book_ticket();
	   void cancel_ticket();
	   void change_reservation();
	   void passenger_details();
	   void get_booking_details();
       void set_flight_info();
       void show_flight_info() const;
       void show_revenue() const;
       void save_data(const string& path = "data.csv");
	   void load_data(const string& path = "data.csv");
       void reset_all();

	private : 

        string flight_no = "AV100";      // default flight number
        string flight_date = "2025-01-01"; // default date
        string origin = "Dhaka";         // default origin
        string destination = "Chittagong"; // default destination
	   //To store details of passenger
	   struct passenger
	   {
		string fname;
		string lname;
		string ID;
		string phone_number;
		string food_menu;
        string seat_class;
		int seat_number;
		int reservation_number;
		passenger *next;
	   };
	   //To denote the start of linked list of passengers
	   passenger *start;

	   passenger *temp_passenger;
	   passenger *temp1;
}flight;	//flight is object of class Flight

// --- helpers for seat selection ---
void print_seat_map() {
    cout << "\nPlease choose another seat number from below:\n";
    for (int x = 1; x <= total_number_of_seats; ++x) {
        if (seats[x - 1] == -1) cout << "|XX|";          // booked
        else                     cout << '|' << setw(2) << x << '|';
        if (x % 10 == 0) cout << '\n';
    }
    cout << '\n';
}

bool is_seat_available(int snumber) {
    if (snumber < 1 || snumber > total_number_of_seats) return false;
    return seats[snumber - 1] != -1;
}

// --- Ticket Pricing System ---
// Calculates total ticket price based on seat class and food choice
int calculate_ticket_price(const std::string& seat_class, const std::string& food_menu) {
    int base_price = 0;

    if (seat_class == "Economy") base_price = 2000;
    else if (seat_class == "Business") base_price = 3500;
    else if (seat_class == "First") base_price = 5000;

    // Add-on for food selection
    if (food_menu == "Veg" || food_menu == "Non-Veg") base_price += 200;

    return base_price;
}

// --- persistence for seat map ---
void save_seat_map(const string& path = "seats.map") {
    ofstream out(path, ios::trunc);
    if (!out) { cout << "Could not write " << path << "\n"; return; }
    // write 100 ints: 0 (free) or -1 (booked)
    for (int i = 0; i < total_number_of_seats; ++i) {
        out << seats[i];
        if (i + 1 < total_number_of_seats) out << ' ';
    }
    out << '\n';
}

void load_seat_map(const string& path = "seats.map") {
    ifstream in(path);
    if (!in) return; // first run — no file yet
    for (int i = 0; i < total_number_of_seats; ++i) {
        if (!(in >> seats[i])) {        // if file shorter/corrupt, default to free
            seats[i] = 0;
        }
    }
}


string repeat(char c, int n) { return string(n, c); }

void sep(int width = 123) { cout << repeat('-', width) << "\n"; }
void print_passenger_header() {
    cout << left
         << setw(15) << "Reservation #"
         << setw(10) << "Seat #"
         << setw(12) << "Class"
         << setw(15) << "First Name"
         << setw(15) << "Last Name"
         << setw(18) << "ID"
         << setw(18) << "Phone Number"
         << setw(10) << "Food" << "\n";
    sep();
}

void print_boxed_menu(const std::string& title,
                      const std::vector<std::string>& items,
                      int requested_width = -1) {
    // 1) Figure out the natural width
    int max_item_len = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        // "  " + digits + ") " + item
        std::ostringstream os;
        os << "  " << (i + 1) << ") " << items[i];
        max_item_len = std::max<int>(max_item_len, (int)os.str().size());
    }
    int title_len = (int)std::string("  ").size() + (int)title.size();

    // Minimum inner width to fit everything
    int natural_width = std::max(title_len, max_item_len);

    // If caller passed a width, respect it; otherwise use natural width (and clamp a bit)
    int inner = (requested_width > 0) ? requested_width : std::max(40, natural_width);
    menu_ui_width = inner;  // <-- store for header to use

    auto border = std::string(inner, '-');

    // Helper to center a line inside the box
    auto center = [&](const std::string& s) {
        int pad = std::max(0, inner - (int)s.size());
        int left = pad / 2, right = pad - left;
        std::cout << '|' << std::string(left, ' ') << s << std::string(right, ' ') << "|\n";
    };

    std::cout << '|' << border << "|\n";
    center("  " + title);
    std::cout << '|' << border << "|\n";
    center("Choose your option:");
    std::cout << '|' << border << "|\n";

    // Items
    for (size_t i = 0; i < items.size(); ++i) {
        std::ostringstream row;
        row << "  " << (i + 1) << ") " << items[i];
        int pad = std::max(0, inner - (int)row.str().size());
        std::cout << '|' << row.str() << std::string(pad, ' ') << "|\n";
    }

    std::cout << '|' << border << "|\n\n";
}


void Flight :: book_ticket()
{
	//To store the details of passenger
	temp_passenger = new passenger;
	cout << "Enter your first name: ";
	cin >> temp_passenger->fname;
	cout << "Enter your last name: ";
	cin >> temp_passenger->lname;
	cout << "Enter your ID: ";
	cin >> temp_passenger->ID;
	cout << "Enter your phone number: ";
	cin >> temp_passenger->phone_number;
	int snumber;

    // seat selection with validation (no taken_seat, no allocate_seat_number)
    while (true) {
    snumber = read_int("Enter the seat number : ", 1, total_number_of_seats);

    if (!is_seat_available(snumber)) {
        cout << "The seat is taken already.\n";
        print_seat_map();
        continue;
    }

    // valid & free → book it
    seats[snumber - 1] = -1;
    temp_passenger->seat_number = snumber;
    cout << "Select seat class:\n"
     << "1. Economy\n"
     << "2. Business\n"
     << "3. First\n";
    int cls = read_int("Your choice: ", 1, 3);
    temp_passenger->seat_class = (cls == 1 ? "Economy" : cls == 2 ? "Business" : "First");
    break;
}



	cout << "Enter your food choice preference : " << endl;
	cout<< "1. Veg" << endl;
	cout<< "2. Non-Veg" << endl;
	cout<< "3. No Food" << endl;
	int choice;
	choice = read_int("Your choice : ", 1, 3);

	if(choice==1)
	{
		temp_passenger->food_menu = "Veg";
	}
	else if(choice==2)
	{
		temp_passenger->food_menu = "Non-Veg";
	}
	else
	{
		temp_passenger->food_menu = "No Food";
	}

    // --- Calculate ticket price ---
    int ticket_price = calculate_ticket_price(temp_passenger->seat_class, temp_passenger->food_menu);
    cout << "\nYour ticket price is: $" << ticket_price << "\n";

    // Ask for confirmation before finalizing booking
    if (!ask_yes_no("Confirm booking for this price?", /*default=*/false)) {
        cout << "Booking cancelled.\n";
        delete temp_passenger;   // avoid leaking the newly allocated node
        return;
    }

	
	reserve_seats++;
	temp_passenger->reservation_number = reserve_seats;
	cout<<"Your reservation number is ::== "<<reserve_seats<<endl;


	temp_passenger->next = NULL;

	//If the linked list is empty
	if(start == NULL)
	{
		start = temp_passenger;
	}
	else
	{
		passenger *temp = start;
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = temp_passenger;
	}
    save_seat_map();
    save_data();
}


void Flight::cancel_ticket()
{
    int reservation_number;
    reservation_number = read_int("Enter your reservation number: ", 1, 1000000000);

    // Fast-fail: nothing booked yet
    if (start == nullptr) {
        cout << "There are no bookings yet.\n";
        return;
        }
    // Quick range sanity check (your numbers start at 1001 and go up)
    if (reservation_number < 1001 || reservation_number > reserve_seats) {
        cout << "Invalid reservation number.\n";
        return;
        }

    temp_passenger = start;
    temp1 = NULL;

    while (temp_passenger != NULL)
    {
        if (temp_passenger->reservation_number == reservation_number)
        {
            // free the correct seat
            int idx = temp_passenger->seat_number - 1;
            if (idx >= 0 && idx < total_number_of_seats) {
                seats[idx] = 0; // mark as free
            }
            // unlink node
            if (temp_passenger == start) {
                start = start->next;
            } else {
                temp1->next = temp_passenger->next;
            }
            delete temp_passenger;
            cancel_tickets++;
            cout << "Ticket cancelled.\n";
            save_seat_map();
            save_data();
            return;
        }
        temp1 = temp_passenger;
        temp_passenger = temp_passenger->next;
    }
    cout << "Reservation not found.\n";
}


void Flight::change_reservation() {
    int currentseat_number, next_seat;

    currentseat_number = read_int("Please enter your current seat number: ", 1, total_number_of_seats);

    // Find the passenger with this seat
    passenger* currentpass = start;
    while (currentpass != NULL && currentpass->seat_number != currentseat_number) {
        currentpass = currentpass->next;
    }

    if (currentpass == NULL) {
        cout << "No reservation found for seat " << currentseat_number << ".\n";
        return;
    }
    cout << "Your current seat: " << currentseat_number << "\n";
    cout << "\nPlease choose another seat number from below:\n";
    // Print seat map (XX = booked), 10 per row
    for (int x = 1; x <= total_number_of_seats; ++x) {
        if (seats[x - 1] == -1) {
            cout << "|XX|";
        } else {
            cout << '|' << setw(2) << x << '|';
        }
        if (x % 10 == 0) cout << '\n';
    }
    cout << '\n';

    // Get the new seat with validation
    while (true) {
    next_seat = read_int("Enter new seat number: ", 1, total_number_of_seats);

        //Prevent choosing the same seat
        if (next_seat == currentseat_number) {
            cout << "You're already seated there. Please choose a different seat.\n";
            continue;
        }

        // Check if already booked
        if (seats[next_seat - 1] == -1) {
            cout << "That seat is already taken. Choose another.\n";
            continue;
        }

        break;
    }

    // Ask for confirmation before actually changing
    if (!ask_yes_no(
        "Confirm changing seat from " + to_string(currentseat_number) +
        " to " + to_string(next_seat) + "?", /*default=*/false)) {
    cout << "Seat change cancelled.\n";
    return;
}

    // Free old seat, set new seat
    seats[currentpass->seat_number - 1] = 0;
    currentpass->seat_number = next_seat;
    seats[next_seat - 1] = -1;

    cout << "Seat changed successfully to " << next_seat << ".\n";
    save_seat_map();
    save_data();
}


void Flight::passenger_details()
{
    int reservation_number = read_int("Enter your reservation number: ", 1, 1'000'000'000);

    // Fast-fail: nothing booked yet
    if (start == nullptr) {
        cout << "There are no bookings yet.\n";
        return;
    }

    // Quick range sanity check
    if (reservation_number < 1001 || reservation_number > reserve_seats) {
        cout << "Invalid reservation number.\n";
        return;
    }
    passenger* p = start;
    while (p && p->reservation_number != reservation_number) p = p->next;

    if (!p) { cout << "Reservation not found.\n"; return; }

    show_flight_info(); // header
    print_passenger_header();

    cout << left
         << setw(15) << p->reservation_number
         << setw(10) << p->seat_number
         << setw(12) << p->seat_class
         << setw(15) << p->fname
         << setw(15) << p->lname
         << setw(18) << p->ID
         << setw(18) << p->phone_number
         << setw(10) << p->food_menu
         << "\n";
}


void Flight::get_booking_details()
{
    show_flight_info();  // header

    if (start == nullptr) {
        cout << "No bookings yet.\n";
        return;
    }

    print_passenger_header();

    passenger* p = start;
    while (p) {
        cout << left
             << setw(15) << p->reservation_number
             << setw(10) << p->seat_number
             << setw(12) << p->seat_class
             << setw(15) << p->fname
             << setw(15) << p->lname
             << setw(18) << p->ID
             << setw(18) << p->phone_number
             << setw(10) << p->food_menu
             << "\n";

        // --- NEW: show ticket price below each passenger ---
        int price = calculate_ticket_price(p->seat_class, p->food_menu);
        cout << "     → Ticket Price: $" << price << "\n";

        p = p->next;

    }
}

// --- Show total revenue from all bookings ---
void Flight::show_revenue() const {
    if (start == nullptr) {
        cout << "No bookings yet.\n";
        return;
    }

    int total = 0;
    passenger* p = start;
    while (p) {
        total += calculate_ticket_price(p->seat_class, p->food_menu);
        p = p->next;
    }

    cout << "\n======================================\n";
    cout << "  Total Revenue from Bookings: $" << total << "\n";
    cout << "======================================\n";
}

// --- Save and load passenger data (persistent storage) ---
void Flight::save_data(const string& path) {
    ofstream fout(path, ios::trunc);
    if (!fout) { cout << "Could not write " << path << "\n"; return; }

    // Header with flight metadata
    fout << "#FLIGHT," << flight_no << ',' << flight_date << ','
         << origin << ',' << destination << '\n';

    // Rows: include seat_class
    for (passenger* p = start; p; p = p->next) {
        fout << p->fname << ','
             << p->lname << ','
             << p->ID << ','
             << p->phone_number << ','
             << p->food_menu << ','
             << p->seat_class << ','   // NEW
             << p->seat_number << ','
             << p->reservation_number << '\n';
    }
}
static inline void rtrim_cr(std::string& s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
}


void Flight::load_data(const string& path) {
    ifstream fin(path);
    if (!fin) return;  // first run

    // clear current list
    start = NULL;
    for (int i = 0; i < total_number_of_seats; ++i) seats[i] = 0;

    string line;
    int max_res = reserve_seats;

    // Optional header
    streampos pos = fin.tellg();
    if (getline(fin, line)) {
        if (line.rfind("#FLIGHT,", 0) == 0) {
            // parse header
            string tmp = line.substr(8);
            stringstream hs(tmp);
            getline(hs, flight_no, ',');       rtrim_cr(flight_no);
            getline(hs, flight_date, ',');     rtrim_cr(flight_date);
            getline(hs, origin, ',');          rtrim_cr(origin);
            getline(hs, destination, ',');     rtrim_cr(destination);
        } else {
            // no header; rewind to parse as row
            fin.clear();
            fin.seekg(pos);
        }
    }

    // Passengers
    while (getline(fin, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        passenger* np = new passenger;

        string seatStr, resStr;
        getline(ss, np->fname, ',');        rtrim_cr(np->fname);
        getline(ss, np->lname, ',');        rtrim_cr(np->lname);
        getline(ss, np->ID, ',');           rtrim_cr(np->ID);
        getline(ss, np->phone_number, ','); rtrim_cr(np->phone_number);
        getline(ss, np->food_menu, ',');    rtrim_cr(np->food_menu);
        getline(ss, np->seat_class, ',');   rtrim_cr(np->seat_class);  // NEW
        getline(ss, seatStr, ',');          rtrim_cr(seatStr);
        getline(ss, resStr, ',');           rtrim_cr(resStr);


        np->seat_number = stoi(seatStr);
        np->reservation_number = stoi(resStr);
        np->next = NULL;

        if (np->seat_number >= 1 && np->seat_number <= total_number_of_seats)
            seats[np->seat_number - 1] = -1;

        if (!start) start = np;
        else {
            passenger* t = start; while (t->next) t = t->next; t->next = np;
        }

        if (np->reservation_number > max_res) max_res = np->reservation_number;
    }
    fin.close();

    reserve_seats = max_res;
}


void Flight::reset_all() {
    // clear linked list
    while (start) {
        passenger* nxt = start->next;
        delete start;
        start = nxt;
    }

    // reset seats and counters
    for (int i = 0; i < total_number_of_seats; ++i) seats[i] = 0;
    reserve_seats = 1000;
    cancel_tickets = 0;

    // rewrite files
    save_seat_map();
    save_data();

    cout << "All data cleared.\n";
}

void Flight::set_flight_info() {
    cout << "Set flight info (leave blank to keep current).\n";

    // Store temporary copies — we’ll only commit if confirmed
    string new_flight_no = flight_no;
    string new_flight_date = flight_date;
    string new_origin = origin;
    string new_destination = destination;

    // --- Flight number ---
    cout << "Flight number [" << flight_no << "]: ";
    {
        string s;
        getline(cin >> ws, s);
        if (!s.empty()) new_flight_no = s;
    }

    // --- Flight date (YYYY-MM-DD) ---
    cout << "Flight date (YYYY-MM-DD) [" << flight_date << "]: ";
    {
    string s;
    getline(cin, s);
    if (!s.empty()) {
        auto is_digit = [](char ch){ return std::isdigit(static_cast<unsigned char>(ch)); };
        bool ok = s.size() == 10
               && is_digit(s[0]) && is_digit(s[1]) && is_digit(s[2]) && is_digit(s[3])
               && s[4] == '-'
               && is_digit(s[5]) && is_digit(s[6])
               && s[7] == '-'
               && is_digit(s[8]) && is_digit(s[9]);

        if (ok) new_flight_date = s;
        else    cout << "Invalid date format. Keeping old value.\n";
        }
    }


    // --- Origin ---
    cout << "Origin [" << origin << "]: ";
    {
        string s;
        getline(cin, s);
        if (!s.empty()) {
            if (s.size() > 1)
                new_origin = s;
            else
                cout << " Origin name too short, keeping old value.\n";
        }
    }

    // --- Destination ---
    cout << "Destination [" << destination << "]: ";
    {
        string s;
        getline(cin, s);
        if (!s.empty()) {
            if (s.size() > 1)
                new_destination = s;
            else
                cout << " Destination name too short, keeping old value.\n";
        }
    }

    // --- Confirmation ---
    cout << "\nReview the new flight details:\n";
    cout << "Flight number: " << new_flight_no << "\n";
    cout << "Date: " << new_flight_date << "\n";
    cout << "Route: " << new_origin << " -> " << new_destination << "\n";

    if (!ask_yes_no("Apply these changes?", /*default=*/false)) {
        cout << "Flight info update cancelled.\n";
        return;
    }

    // Apply updates only if confirmed
    flight_no    = new_flight_no;
    flight_date  = new_flight_date;
    origin       = new_origin;
    destination  = new_destination;

    cout << " Flight info updated successfully.\n";

    save_data();        // writes data.csv (header line contains flight info)
    save_seat_map();    // optional: harmless; keeps both files written together

}


void Flight::show_flight_info() const {
    std::string info = "Flight: " + flight_no +
                       "   Date: " + flight_date +
                       "   Route: " + origin + " -> " + destination;

    int width = std::max(40, menu_ui_width); // use the same width as the menu
    int pad = std::max(0, width - (int)info.size());
    int left = pad / 2, right = pad - left;

    std::cout << "\n" << std::string(left, ' ') << info << std::string(right, ' ') << "\n";
    std::cout << std::string(width, '-') << "\n\n";
}

void welcome() {
    // initial header + menu
    flight.show_flight_info();

    std::vector<std::string> opts = {
        "BOOK TICKET",
        "CANCEL TICKET",
        "CHANGE RESERVATION",
        "PASSENGER DETAILS",
        "GET BOOKING DETAILS",
        "FLIGHT INFO (VIEW/EDIT)",
        "VIEW REVENUE",
        "RESET ALL DATA",
        "EXIT"
    };

    print_boxed_menu("WELCOME TO AEROVERSE AIRLINE FLIGHT RESERVATION SYSTEM", opts);

    while (true) {
        int choice = read_int("Enter your choice: ", 1, 9);

        switch (choice) {
            case 1: clear_screen(); flight.book_ticket(); break;
            case 2: clear_screen(); flight.cancel_ticket(); break;
            case 3: clear_screen(); flight.change_reservation(); break;
            case 4: clear_screen(); flight.passenger_details(); break;
            case 5: clear_screen(); flight.get_booking_details(); break;
            case 6: clear_screen(); flight.set_flight_info(); break;
            case 7: clear_screen(); flight.show_revenue(); break;
            case 8:clear_screen();
                if (ask_yes_no("This will erase ALL data. Continue?", /*default=*/false))
                    flight.reset_all();
                    break;
            case 9: clear_screen(); return;  // EXIT

        }
        // re-show header + menu after each action
        flight.show_flight_info();
        print_boxed_menu("WELCOME TO AEROVERSE AIRLINE FLIGHT RESERVATION SYSTEM", opts);
    }
}

int main() 
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    load_seat_map();
    flight.load_data();
    welcome();
    flight.save_data();
    save_seat_map();
    return 0;
}
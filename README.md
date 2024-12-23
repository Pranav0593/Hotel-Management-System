# Hotel Management System

This project simulates a hotel environment using Operating Systems concepts like process creation, inter-process communication (IPC), and POSIX-compliant C programming. The system represents hotel entities (Admin, Manager, Tables, Customers, and Waiters) as separate processes interacting with each other.

---

## Features

### Process Interactions
- **Admin Process**  
  - Continuously monitors the hotel's status and takes user input to decide whether to close the hotel.  
  - Initiates hotel closure by notifying the Hotel Manager.  

- **Hotel Manager Process**  
  - Handles total earnings, waiter wages, and overall profit calculation.  
  - Writes the earnings for each table to an `earnings.txt` file and displays the total summary when the hotel closes.  
  - Ensures the hotel shuts down only when no customers are present.

- **Table Processes**  
  - Represent tables in the hotel.  
  - Manage customers (child processes), who order food items from a predefined menu (`menu.txt`).  
  - Use pipes for communication with customers and shared memory for communication with waiters.  
  - Allow multiple cycles of seating new customers until termination.  

- **Waiter Processes**  
  - Dedicated to a specific table and responsible for validating orders, calculating bills, and sharing earnings with the Manager.  
  - Communicate with Table processes using shared memory.  

- **Customer Processes**  
  - Created as child processes of Tables to represent customers sitting at the table.  
  - Allow customers to order food using a menu-driven interface.  
  - Handle invalid orders gracefully, requiring customers to reorder.  

---

### IPC Mechanisms
- **Pipes:** Used between Table and Customer processes for sending orders.  
- **Shared Memory:** Used between Table and Waiter processes, and between Waiter and Manager processes, for efficient data exchange.  

### Menu and Billing System
- Predefined menu (`menu.txt`) includes items with prices.
- Orders are placed by customers and processed by Waiters.
- Total earnings are calculated and logged to `earnings.txt`.

### Termination Workflow
- Admin initiates termination when required.  
- The Hotel Manager ensures all customer and waiter processes terminate cleanly before shutting down the system.  
- IPC mechanisms are cleaned up during the shutdown.

---

## Implementation Details

### Constraints
1. **POSIX Compliance:** Code is written in POSIX-compliant C and runs on Ubuntu 22.04.  
2. **Limited Table and Waiter Processes:** Maximum of 10 Table and 10 Waiter processes, each uniquely mapped (Table 1 ↔ Waiter 1, and so on).  
3. **Sequential Execution:** Tables and Waiters can be launched incrementally, allowing dynamic addition of processes.  

### File Structure
- `table.c`: Code for creating and managing Table processes.  
- `waiter.c`: Code for creating Waiter processes.  
- `hotelmanager.c`: Code for the Hotel Manager process.  
- `admin.c`: Code for the Admin process.  
- `menu.txt`: Contains the predefined menu with item names and prices.  
- `earnings.txt`: Logs earnings for each table and the final summary.  

### Error Handling
- Validates customer inputs for orders (e.g., serial numbers of menu items).  
- Handles process terminations safely to avoid resource leaks.  
- Ensures unused pipe ends are closed appropriately.

---

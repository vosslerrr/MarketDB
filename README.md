# MarketDB

| [Prequisites](#prerequisites) - [Getting Started](#getting-started) - [Submitting Entries](#submitting-entries) - [Deleting Entries](#deleting-entries) - [Modifying Entries](#modifying-entries) - [Search Function](#search-function)
:----------------------------------------------------------: |
| [DDL Statements](#ddl-statements) - [Entity Relationship Diagram](#entity-relationship-diagram) |

## Tutorial

### Prerequisites

This application requires Microsoft’s ODBC x86 driver in order to connect with the MySQL Server. This driver can be found at https://go.microsoft.com/fwlink/?linkid=2306655. 
More information about the Microsoft ODBC driver can be found at https://learn.microsoft.com/en-us/sql/connect/odbc/microsoft-odbc-driver-for-sql-server?view=sql-server-ver16. 
If installation is successful, the application will launch with no errors. If driver was installed incorrectly, or if the incorrect driver was downloaded the following window will display upon launch of the application (Fig. 1).

<p align="center">
  <img width="400" height="235" alt="image" src="https://github.com/user-attachments/assets/bf593d48-a4ec-4edb-b071-e323d1d23fd3" />
</p>

<p align="center">
  (Figure 1. Missing ODBC Driver error window)
</p>

Basic knowledge of how to setup a MySQL server and how to create a database within your server is needed. For my information visit https://www.mysql.com/. 
This application is compatible with version 8.0.42 MySQL Installer.

___

### Getting Started

Upon launch of the application the following window will appear (Fig. 2)

<p align="center">
  <img width="975" height="767" alt="image" src="https://github.com/user-attachments/assets/324d1e11-852c-44ad-b8b9-4d58f6709bed" />
</p>

<p align="center">
  (Figure 2. MarketDB Login Window)
</p>

All required login information (with the exception of the password) can be found with the launch of your MySQL Workbench, see figure 3 and figure 4 for more details on where to find the information.

<p align="center">
  <img width="813" height="496" alt="image" src="https://github.com/user-attachments/assets/c49a7fa0-c8bc-4203-b2fb-ccabc614ca98" />
</p>

<p align="center">
  (Figure 3. MySQL Workbench)
</p>

<p align="center">
  <img width="847" height="519" alt="image" src="https://github.com/user-attachments/assets/858d6623-0ccb-444f-8cfc-c5b3a04440f5" />
</p>

<p align="center">
  (Figure 4. MySQL Database)
</p>

The MySQL password for the server is specified upon MySQL installation and is not stored within MarketDB, the MySQL server password is only used once upon connection. Users are responsible for their own password management.

If connection is unsuccessful, the following error message appears (Fig. 5): 

<p align="center">
  <img width="464" height="267" alt="image" src="https://github.com/user-attachments/assets/1584da8b-a4eb-4163-b152-a0ae167e8aff" />
</p>

<p align="center">
  (Figure 5. Connection Failed! error window)
</p>

If connection is successful, the login window will close and the MarketDB home screen will appear and the user will be ready to use MarketDB.

Upon successful connection, if the required tables are not setup inside of the database, the application automatically creates all required tables needed for full function. 
See the MarketDB ERD and DDL Create Statements.txt for specific table requirements.

___

### Submitting Entries

Upon successful login, the following home screen is displayed (Fig. 6):

<p align="center">
  <img width="975" height="569" alt="image" src="https://github.com/user-attachments/assets/ee2883e0-2988-4a42-be0f-53b236781389" />
</p>

<p align="center">
  (Figure 6. MarketDB Home Screen)
</p>

Users can now select one of the five tables at the top of the window to make an entry into. Or users can click the magnifying glass icon in the top right of the window to search through the tables based on a specified selection. 
For this section of the tutorial, we will be making an entry into the Item table.

The Item selection displays the following screen (Fig. 7): 

<p align="center">
  <img width="975" height="570" alt="image" src="https://github.com/user-attachments/assets/c8e90799-4f2a-4e93-bada-aeb45b860ea4" />
</p>

<p align="center">
  (Figure 7. Item Screen)
</p>

Located on the right side of the window is a table listing every entry that has been entered into the item table in MySQL. Users can interact with the table by clicking the column names to sort the entries alphabetically or reverse alphabetically. 
There are also two buttons located at the bottom of the table entries that will be further discussed later on in the manual. Rows in the table can be highlighted by hovering over an entry and left clicking. 
Located on the left side of the window are the input text boxes that are used to take in user input and commit to the MySQL database when the submit button is pressed. 

In order to submit an entry, certain conditions must be met. These conditions might not be obvious at first, but upon clicking the ‘Submit’ button, all inputs that are incorrect will be highlighted and the errors explained. 
For empty or invalid inputs, the following window will appear (Fig. 8):

<p align="center">
  <img width="520" height="267" alt="image" src="https://github.com/user-attachments/assets/1ea8fa9f-573c-42bd-a053-ec3a12c3968d" />
</p>

<p align="center">
  (Figure 8. Invalid/Empty Inputs! error window)
</p>

All input boxes that have an error associated with them will be highlighted red. If an error message still occurs when all boxes have text in them, that means that there is a type mismatch in one of the boxes (string to int/float). 
Please refer to the MarketDB ERD for specific types related to entity attributes.

Another condition that must be met is unique primary keys. If a user tries to enter an Item I.D. (for this example int the item table) that is already in use, the following error message will appear (Fig. 9): 

<p align="center">
  <img width="347" height="246" alt="image" src="https://github.com/user-attachments/assets/6ee2b0b1-6c52-4c27-a1d3-2161109ca46a" />
</p>

<p align="center">
  (Figure 9. Duplicate Primary Key! error window)
</p>

The primary key is always shown as the first column in the table located on the right side of the window for each entity (Item, Aisle, Section, etc.). Refer to the MarketDB ERD for exact primary key names.

The last condition that must be met are foreign key constraints. If a user tries to enter an Aisle No or Section I.D. (for this example in the item table) that does not exist, the following error messages will appear (Fig 10., Fig 11.): 

<p align="center">
  <img width="357" height="245" alt="image" src="https://github.com/user-attachments/assets/1f272353-f9e5-4d49-984d-06c953fc893c" />
</p>

<p align="center">
  (Figure 10. Foreign Key Constraint! (Section I.D.) error window)
</p>

<p align="center">
  <img width="337" height="245" alt="image" src="https://github.com/user-attachments/assets/e81991ba-f30b-4db9-811b-6287e4b84ba7" />
</p>

<p align="center">
  (Figure 11. Foreign Key Constraint! (Aisle No.) error window)
</p>

For specific foreign key constraints please refer to the MarketDB ERD and DDL Create Statements.txt.

Assuming all condition are met for a successful entry, pressing the submit button will automatically refresh the table shown in the window and the entry will be inserted into the MySQL database table.

___

### Deleting Entries

Deleting an entry is very easy in MarketDB. In order to delete an entry, the user must select an entry from the table shown in the window like so (Fig. 12): 

<p align="center">
  <img width="975" height="568" alt="image" src="https://github.com/user-attachments/assets/cfea8751-1af0-412d-89c5-bc34542bfd0c" />
</p>

<p align="center">
  (Figure 12. Highlighted entry)
</p>

Once an entry has been selected, the row will be highlighted blue to indicate selection. Then the user left clicks the delete button located at the bottom of the table and the entry will be deleted from the table and from the MySQL database table.

___

### Modifying Entries

In order to modify an entry inside of MarketDB, the user must first select an entry from the table the same way the user selected an entry to delete earlier in the manual (Page 11). 
Once an entry is selected, the user must left click the Modify button located at the bottom of the table, next to the Delete button. 
Once the modify button is clicked, all of the entry's attributes will be put into the input text boxes on the left side of the screen like so (Fig. 13): 

<p align="center">
  <img width="975" height="568" alt="image" src="https://github.com/user-attachments/assets/08d52efd-357e-48fc-88a8-b44343a364f6" />
</p>

<p align="center">
  (Figure 13. Modify Button effects)
</p>

A modify submission does not meet the same conditions as a regular submission. The primary key may stay the same as it is currently displayed if the user wishes. 
All other conditions are the same as a regular submission however. Please refer to page 9 of this manual for submission conditions. 

___

### Search Function

Located at the top right of the MarketDB window is a magnifying glass. Upon left-clicking, a window will be displayed on the right side of the screen, prompting the user to select a condition to search by (Fig. 14).

<p align="center">
  <img width="975" height="571" alt="image" src="https://github.com/user-attachments/assets/65d4b0cf-691a-4b19-9f54-0ab0ae137631" />
</p>

<p align="center">
  (Figure 14. Search Screen)
</p>

Once a user makes a selection from the screen, an input is needed in box directly below it. If the input is empty or the input does not exist with the selection made, the following error messages will display (Fig. 15, Fig 16.):

<p align="center">
  <img width="336" height="244" alt="image" src="https://github.com/user-attachments/assets/a480c4fd-75c8-4989-9433-6180852c7038" />
</p>

<p align="center">
  (Figure 15. Invalid Item I.D.! error message)
</p>

<p align="center">
  <img width="271" height="241" alt="image" src="https://github.com/user-attachments/assets/32712012-d401-417c-93e2-ef1df8e55999" />
</p>

<p align="center">
  (Figure 16. Empty Input! error message)
</p>

Assuming that all conditions have been met and the ‘Go’ button has been clicked, a successful search query will result in a display of all tables that have a relation with the selection and the input. For this example we will show a successful search with an Item I.D. (Fig 17.): 

<p align="center">
  <img width="975" height="574" alt="image" src="https://github.com/user-attachments/assets/64c72f7d-b220-44dc-9299-9e5c83d8ed08" />
</p>

<p align="center">
  (Figure 17. Successful Search Results)
</p>

___

### DDL Statements

These create statements are also located inside of main.cpp
Create statements are ripped directly from MySQL DDL tab

```
CREATE TABLE `item` (
  `item_id` varchar(45) NOT NULL,
  `item_name` varchar(45) NOT NULL,
  `aisle_no` int NOT NULL,
  `section_id` varchar(45) NOT NULL,
  `item_price` float NOT NULL,
  `no_of_items` int NOT NULL,
  PRIMARY KEY (`item_id`),
  KEY `fk_item_aisle` (`aisle_no`),
  KEY `fk_item_section` (`section_id`),
  CONSTRAINT `fk_item_aisle` FOREIGN KEY (`aisle_no`) REFERENCES `aisle` (`aisle_no`),
  CONSTRAINT `fk_item_section` FOREIGN KEY (`section_id`) REFERENCES `section` (`section_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `aisle` (
  `aisle_no` int NOT NULL,
  `no_of_sections` int NOT NULL,
  PRIMARY KEY (`aisle_no`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `section` (
  `section_id` varchar(45) NOT NULL,
  `section_name` varchar(45) NOT NULL,
  `aisle_no` int NOT NULL,
  PRIMARY KEY (`section_id`),
  KEY `aisle_no_idx` (`aisle_no`),
  CONSTRAINT `fk_section_aisle` FOREIGN KEY (`aisle_no`) REFERENCES `aisle` (`aisle_no`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `supplier` (
  `supplier_id` varchar(45) NOT NULL,
  `item_id` varchar(45) NOT NULL,
  `item_cost` float NOT NULL,
  `supplier_name` varchar(45) NOT NULL,
  PRIMARY KEY (`supplier_id`),
  KEY `fk_supplier_item` (`item_id`),
  CONSTRAINT `fk_supplier_item` FOREIGN KEY (`item_id`) REFERENCES `item` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `transaction` (
  `transaction_id` varchar(45) NOT NULL,
  `item_id` varchar(45) NOT NULL,
  `item_price` float NOT NULL,
  `tax_amount` float NOT NULL,
  `transaction_total` float NOT NULL,
  `transaction_date` varchar(45) NOT NULL,
  `receipt_no` int NOT NULL,
  PRIMARY KEY (`transaction_id`),
  KEY `fk_transaction_item` (`item_id`),
  CONSTRAINT `fk_transaction_item` FOREIGN KEY (`item_id`) REFERENCES `item` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
```

___

### Entity Relationship Diagram

<img width="1280" height="1280" alt="MarketDB ERD Diagram" src="https://github.com/user-attachments/assets/b1a06197-f6ce-4a2f-a660-ada275ed276b" />

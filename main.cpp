#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <stack>
#include <algorithm>

//Класс Товара
class Product {
public:
    std::string id;
    std::string name;
    double volume;
    std::vector<std::string> composition;

    Product() : volume(0.0) {}
    Product(std::string _id, std::string _name, double _volume, std::vector<std::string> _composition)
        : id(_id), name(_name), volume(_volume), composition(_composition) {
    }
    void print() const {
        std::cout << "ID: " << id << ", Name: " << name << ", Volume: " << volume << ", Composition: ";
        for (auto& c : composition)
            std::cout << c << " ";
        std::cout << std::endl;
    }
};

//Адрес
struct Address {
    std::string index, city, street, house;
};

//Склада
class Warehouse {
public:
    std::string id, name;
    Address address;
    double capacity;
    std::unordered_map<std::string, int> stock;
    Warehouse() : capacity(0.0) {}
    Warehouse(std::string _id, std::string _name, Address _address, double _capacity)
        : id(_id), name(_name), address(_address), capacity(_capacity) {
    }
};

//Продавец
class Seller {
public:
    std::string name, login, password_hash, seller_code;
    std::vector<std::string> warehouse_ids;
    std::unordered_map<std::string, double> products; // id -> price

    Seller() {}
    Seller(std::string _name, std::string _login, std::string _password_hash, std::string _seller_code)
        : name(_name), login(_login), password_hash(_password_hash), seller_code(_seller_code) {
    }
};

//Корзина
class Cart {
public:
    struct CartItem {
        std::string product_id;
        int quantity;
        double price;
    };
    std::vector<CartItem> items;

    void addItem(const std::string& pid, int qty, double p) {
        items.push_back({ pid, qty, p });
    }
    void removeLast() {
        if (!items.empty()) items.pop_back();
    }
    double total() {
        double sum = 0;
        for (auto& item : items)
            sum += item.quantity * item.price;
        return sum;
    }
};

std::string simpleHash(const std::string& str);

//поиск товаров по названию
void searchProductByName(const std::vector<Product>& products, const std::string& name);

// Загрузка логинов
std::unordered_map<std::string, std::string> load_logins(const std::string& filename);

void save_login(const std::string& filename, const std::string& login, const std::string& hash_pw);

// Загрузка товаров
std::vector<Product> loadProducts(const std::string& filename);

// Сохранение товаров
void saveProducts(const std::vector<Product>& products, const std::string& filename);

//Загрузка продавцов
void loadSellers(const std::string& filename, std::unordered_map<std::string, Seller>& sellers);

// Сохранение продавцов
void saveSellers(const std::string& filename, const std::unordered_map<std::string, Seller>& sellers);

//Регистрация продавца
void registerSeller(std::unordered_map<std::string, Seller>& sellers, std::unordered_map<std::string, std::string>& logins);

//Регистрация покупателя
void registerBuyer(std::unordered_map<std::string, std::string>& logins);

void saveCheck(const Cart& cart, const Seller& seller, const std::string& buyer_name);

// --- Основная функция ---
int main() {
    setlocale(LC_ALL, "ru");
    std::unordered_map<std::string, Seller> sellers; // логин -> продавец
    std::vector<Product> products = loadProducts("products.csv");
    std::unordered_map<std::string, std::string> logins = load_logins("logins.txt");

    //Загружаем продавцов из файла
    loadSellers("sellers.txt", sellers);

    int main_choice;
    while (true) {
        std::cout << "\n=== Главное меню ===\n";
        std::cout << "0. Выход\n";
        std::cout << "1. Войти как покупатель\n";
        std::cout << "2. Войти как продавец\n";
        std::cout << "Выберите: ";
        std::cin >> main_choice;
        if (main_choice == 0) break;

        if (main_choice == 1) {
            int sub_choice;
            std::cout << "1. Войти\n2. Зарегистрироваться\nВыберите: ";
            std::cin >> sub_choice;
            if (sub_choice == 2) {
                registerBuyer(logins);
                continue; // снова в главное меню
            }
            //Вход покупателя
            std::string buyer_name, login, pass;
            std::cout << "Введите имя покупателя: ";
            std::cin >> buyer_name;
            std::cout << "Введите логин: ";
            std::cin >> login;
            std::cout << "Введите пароль: ";
            std::cin >> pass;
            auto it = logins.find(login);
            if (it == logins.end() || simpleHash(pass) != it->second) {
                std::cout << "Неправильный логин или пароль.\n";
                continue;
            }
            //Вход успешен
            std::stack<Cart> cartStack;
            Cart currentCart;

            int buyer_menu;
            do {
                std::cout << "\n--- Меню покупателя ---\n";
                std::cout << "1. Поиск товара по названию\n";
                std::cout << "2. Посмотреть все товары продавца\n"; // тут можно выбрать продавца (для упрощения - все продавцы)
                std::cout << "3. Добавить товар в корзину\n";
                std::cout << "4. Удалить последний товар из корзины\n";
                std::cout << "5. Оформить покупку (получить чек)\n";
                std::cout << "0. Назад\n";
                std::cout << "Выберите: ";
                std::cin >> buyer_menu;
                if (buyer_menu == 0) break;

                if (buyer_menu == 1) {
                    std::string name_search;
                    std::cout << "Введите название товара для поиска: ";
                    std::cin >> name_search;
                    searchProductByName(products, name_search);
                }
                else if (buyer_menu == 2) {
                    //показываем все товары всех продавцов
                    for (auto& s : sellers) {
                        std::cout << "Продавец: " << s.second.name << "\n";
                        for (auto& p : products) {
                            if (s.second.products.count(p.id))
                                std::cout << "ID: " << p.id << ", Name: " << p.name << ", Price: " << s.second.products.at(p.id) << "\n";
                        }
                    }
                }
                else if (buyer_menu == 3) {
                    std::string prod_id;
                    int qty;
                    std::cout << "Введите ID товара: ";
                    std::cin >> prod_id;
                    std::cout << "Введите количество: ";
                    std::cin >> qty;

                    //ищем товар
                    auto itp = std::find_if(products.begin(), products.end(), [&](const Product& p) {
                        return p.id == prod_id;
                        });
                    if (itp == products.end()) {
                        std::cout << "Товар не найден.\n";
                        continue;
                    }
                    //ищем продавца, у которого есть товар
                    bool found = false;
                    for (auto& s : sellers) {
                        if (s.second.products.count(prod_id)) {
                            double price = s.second.products.at(prod_id);
                            currentCart.addItem(prod_id, qty, price);
                            std::cout << "Товар добавлен в корзину.\n";
                            found = true;
                            break;
                        }
                    }
                    if (!found) std::cout << "Товар у продавца не найден.\n";
                }
                else if (buyer_menu == 4) {
                    currentCart.removeLast();
                    std::cout << "Последний товар удален из корзины.\n";
                }
                else if (buyer_menu == 5) {
                    //Оформляем покупку
                    double sum = currentCart.total();
                    std::cout << "Общая сумма покупки: " << sum << "\n";
                    //Генерация чека
                    //Перед этим сохраняем чек
                    if (!sellers.empty()) {
                        auto& firstSeller = sellers.begin()->second;
                        saveCheck(currentCart, firstSeller, buyer_name);
                    }
                    //Очищаем корзину
                    currentCart.items.clear();
                }

            } while (true);
        }
        else if (main_choice == 2) {
            //Вход продавца
            int sub_choice;
            std::cout << "1. Войти\n2. Зарегистрироваться\nВыберите: ";
            std::cin >> sub_choice;
            if (sub_choice == 2) {
                registerSeller(sellers, logins);
                continue; //снова в главное меню
            }
            //Вход продавца
            std::string login, pass;
            std::cout << "Введите логин: ";
            std::cin >> login;
            std::cout << "Введите пароль: ";
            std::cin >> pass;

            auto sellerIt = sellers.find(login);
            if (sellerIt == sellers.end()) {
                std::cout << "Вы не зарегистрированы как продавец.\n";
                continue;
            }
            //Проверка пароля
            if (sellerIt->second.password_hash != simpleHash(pass)) {
                std::cout << "Некорректный пароль.\n";
                continue;
            }
            //Успешный вход
            Seller& currentSeller = sellerIt->second;
            //Отладка: вывод зарегистрированных продавцов
            std::cout << "Зарегистрированные продавцы:\n";
            for (const auto& s : sellers) {
                std::cout << "Логин: " << s.first << ", Имя: " << s.second.name << "\n";
            }

            int seller_choice;
            do {
                std::cout << "\n--- Меню продавца ---\n";
                std::cout << "1. Посмотреть все товары\n";
                std::cout << "2. Добавить новый товар\n";
                std::cout << "3. Изменить цену товара\n";
                std::cout << "0. Назад\n";
                std::cout << "Выберите: ";
                std::cin >> seller_choice;
                if (seller_choice == 0) break;

                if (seller_choice == 1) {
                    // показать все товары продавца
                    std::cout << "Товары:\n";
                    for (auto& p : products) {
                        auto itp = currentSeller.products.find(p.id);
                        if (itp != currentSeller.products.end()) {
                            std::cout << "ID: " << p.id << ", Name: " << p.name
                                << ", Price: " << itp->second << "\n";
                        }
                    }
                }
                else if (seller_choice == 2) {
                    // добавляем товар
                    std::string prod_id, prod_name;
                    double price;
                    std::cout << "Введите ID нового товара: ";
                    std::cin >> prod_id;
                    std::cout << "Введите название: ";
                    std::cin >> prod_name;
                    std::cout << "Введите цену: ";
                    std::cin >> price;
                    //создать и добавить товар
                    Product newProd(prod_id, prod_name, 0.0, {}); // volume - 0, состав - пусто
                    products.push_back(newProd);
                    currentSeller.products[prod_id] = price;
                    std::cout << "Товар добавлен.\n";
                }
                else if (seller_choice == 3) {
                    // изменить цену товара
                    std::string prod_id;
                    double new_price;
                    std::cout << "Введите ID товара: ";
                    std::cin >> prod_id;
                    std::cout << "Введите новую цену: ";
                    std::cin >> new_price;
                    if (currentSeller.products.count(prod_id)) {
                        currentSeller.products[prod_id] = new_price;
                        std::cout << "Цена обновлена.\n";
                    }
                    else {
                        std::cout << "Товар не найден.\n";
                    }
                }

            } while (true);
        }
        else {
            std::cout << "Неверный выбор.\n";
        }
    }

    //сохранение продавцов
    saveSellers("sellers.txt", sellers);

    return 0;
}

std::string simpleHash(const std::string& str) {
    std::string hash = str;
    char key = 0x5A;
    for (auto& c : hash) c ^= key;
    return hash;
}
//Реализация функции поиска товаров по названию
void searchProductByName(const std::vector<Product>& products, const std::string& name) {
    bool found = false;
    for (const auto& p : products) {
        if (p.name.find(name) != std::string::npos) { // ищем подстроку
            p.print();
            found = true;
        }
    }
    if (!found) {
        std::cout << "Товары не найдены.\n";
    }
}

std::unordered_map<std::string, std::string> load_logins(const std::string& filename) {
    std::unordered_map<std::string, std::string> login_map;
    std::ifstream fin(filename);
    std::string line;
    while (getline(fin, line)) {
        size_t delim = line.find('|');
        if (delim != std::string::npos) {
            login_map[line.substr(0, delim)] = line.substr(delim + 1);
        }
    }
    return login_map;
}

void save_login(const std::string& filename, const std::string& login, const std::string& hash_pw) {
    std::ofstream fout(filename, std::ios::app);
    fout << login << "|" << hash_pw << "\n";
}

std::vector<Product> loadProducts(const std::string& filename) {
    std::vector<Product> products;
    std::ifstream fin(filename);
    std::string line, token;
    if (!fin.is_open()) return products;
    getline(fin, line); // заголовки
    while (getline(fin, line)) {
        std::stringstream ss(line);
        std::string id, name, volume_str, composition_str;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, volume_str, ',');
        getline(ss, composition_str, ',');
        double volume = atof(volume_str.c_str());
        std::vector<std::string> composition;
        std::stringstream comp_ss(composition_str);
        while (getline(comp_ss, token, ';')) {
            composition.push_back(token);
        }
        products.emplace_back(id, name, volume, composition);
    }
    return products;
}

void saveProducts(const std::vector<Product>& products, const std::string& filename) {
    std::ofstream fout(filename);
    fout << "id,name,volume,composition\n";
    for (auto& p : products) {
        fout << p.id << "," << p.name << "," << p.volume << ",";
        for (size_t i = 0; i < p.composition.size(); ++i) {
            fout << p.composition[i];
            if (i != p.composition.size() - 1) fout << ";";
        }
        fout << "\n";
    }
}

void loadSellers(const std::string& filename, std::unordered_map<std::string, Seller>& sellers) {
    std::ifstream fin(filename);
    if (!fin.is_open()) return; // если файла нет, ничего не делаем
    std::string line;
    while (getline(fin, line)) {
        std::stringstream ss(line);
        std::string login, pw_hash, name, seller_code, warehouses_str, prices_str;

        if (getline(ss, login, '|') &&
            getline(ss, pw_hash, '|') &&
            getline(ss, name, '|') &&
            getline(ss, seller_code, '|') &&
            getline(ss, warehouses_str, '|') &&
            getline(ss, prices_str)) {

            Seller s(name, login, pw_hash, seller_code);
            // Разделяем warehouse_ids
            std::stringstream ws(warehouses_str);
            std::string warehouse_id;
            while (getline(ws, warehouse_id, ',')) {
                s.warehouse_ids.push_back(warehouse_id);
            }
            // Разделяем цены
            std::stringstream ps(prices_str);
            std::string item;
            while (getline(ps, item, ';')) {
                size_t delim = item.find(':');
                if (delim != std::string::npos) {
                    std::string prod_id = item.substr(0, delim);
                    double price = atof(item.substr(delim + 1).c_str());
                    s.products[prod_id] = price;
                }
            }
            sellers[login] = s;
        }
    }
}

void saveSellers(const std::string& filename, const std::unordered_map<std::string, Seller>& sellers) {
    std::ofstream fout(filename);
    for (const auto& pair : sellers) {
        const Seller& s = pair.second;
        // собираем warehouse_ids
        std::string warehouses_str;
        for (size_t i = 0; i < s.warehouse_ids.size(); ++i) {
            warehouses_str += s.warehouse_ids[i];
            if (i != s.warehouse_ids.size() - 1) warehouses_str += ",";
        }
        // собираем prices
        std::string prices_str;
        for (auto& kv : s.products) {
            prices_str += kv.first + ":" + std::to_string(kv.second) + ";";
        }
        // удаляем последний ';'
        if (!prices_str.empty()) prices_str.pop_back();

        fout << s.login << "|" << s.password_hash << "|" << s.name << "|" << s.seller_code << "|"
            << warehouses_str << "|" << prices_str << "\n";
    }
}

void registerSeller(std::unordered_map<std::string, Seller>& sellers, std::unordered_map<std::string, std::string>& logins) {
    std::string login, password, name, seller_code;
    std::cout << "Введите логин для нового продавца: ";
    std::cin >> login;
    if (sellers.count(login) || logins.count(login)) {
        std::cout << "Этот логин уже занят.\n";
        return;
    }
    std::cout << "Введите пароль: ";
    std::cin >> password;
    std::cout << "Введите название магазина: ";
    std::cin >> name;
    std::cout << "Введите код продавца: ";
    std::cin >> seller_code;
    std::string hash_pw = simpleHash(password);
    Seller newSeller(name, login, hash_pw, seller_code);
    sellers[login] = newSeller;
    save_login("seller.txt", login, hash_pw);
    std::cout << "Продавец успешно зарегистрирован.\n";
}

void registerBuyer(std::unordered_map<std::string, std::string>& logins) {
    std::string login, password, name;
    std::cout << "Введите логин для нового покупателя: ";
    std::cin >> login;
    if (logins.count(login)) {
        std::cout << "Этот логин уже занят.\n";
        return;
    }
    std::cout << "Введите пароль: ";
    std::cin >> password;
    std::cout << "Введите ваше имя: ";
    std::cin >> name;
    std::string hash_pw = simpleHash(password);
    save_login("logins.txt", login, hash_pw);
    logins[login] = hash_pw;
    std::cout << "Покупатель успешно зарегистрирован.\n";
}

//Генерация чеков
void saveCheck(const Cart& cart, const Seller& seller, const std::string& buyer_name) {
    auto t = std::time(nullptr);
    struct tm tm {};
    localtime_s(&tm, &t);
    std::stringstream filename;
    filename << "check_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".txt";

    std::ofstream fout(filename.str());
    fout << "Дата: " << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "\n";
    fout << "Покупатель: " << buyer_name << "\n";
    fout << "Продавец: " << seller.name << "\n";
    fout << "Товары:\n";

    double total_sum = 0;
    for (auto& item : cart.items) {
        double sum = item.quantity * item.price;
        total_sum += sum;
        fout << "Product ID: " << item.product_id << ", Quantity: " << item.quantity
            << ", Price: " << item.price << ", Sum: " << sum << "\n";
    }
    fout << "Общая сумма: " << total_sum << "\n";
}
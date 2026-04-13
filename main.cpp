#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <clocale>

using namespace std;
using namespace chrono;

struct User {
    int id;
    string name;
    int age;
    string email;
    
    User() : id(-1), name(""), age(0), email("") {}
    
    User(int i, string n, int a, string e) 
        : id(i), name(n), age(a), email(e) {}
};

class InformationModel {
private:
    vector<User> users;
    unordered_map<string, User> nameCache;
    
    long long cacheHits;
    long long cacheMisses;
    long long linearSearches;
    bool silentMode;
    
public:
    InformationModel() : cacheHits(0), cacheMisses(0), linearSearches(0), silentMode(false) {
        loadLargeTestData();
    }
    
    void loadLargeTestData() {
        const int RECORD_COUNT = 500000;
        cout << "Загрузка " << RECORD_COUNT << " пользователей..." << endl;
        
        users.reserve(RECORD_COUNT);
        for (int i = 1; i <= RECORD_COUNT; i++) {
            string name = "User_" + to_string(i);
            users.push_back(User(i, name, 20 + (i % 50), name + "@mail.ru"));
        }
        
        cout << "Загружено " << users.size() << " пользователей." << endl;
    }
    
    User findByNameLinear(const string& name) {
        linearSearches++;
        for (const auto& user : users) {
            if (user.name == name) {
                return user;
            }
        }
        return User();
    }
    
    User findByNameWithCache(const string& name, bool enableOutput = true) {
        auto it = nameCache.find(name);
        if (it != nameCache.end()) {
            cacheHits++;
            return it->second;
        }
        
        cacheMisses++;
        if (!silentMode && enableOutput) 
            cout << "  [Промах] Линейный поиск..." << endl;
        
        User result = findByNameLinear(name);
        
        if (result.id != -1) {
            nameCache[name] = result;
            if (!silentMode && enableOutput) 
                cout << "  [Сохранение] Добавлен в кэш" << endl;
        }
        
        return result;
    }
    
    void performanceComparison() {
        cout << "\n=== СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===" << endl;
        
        string testNames[] = {"User_1", "User_2", "User_3", "User_4", "User_5"};
        const int ITERATIONS = 1000000;  // 1 МИЛЛИОН запросов!
        
        silentMode = true;
        
        // ТЕСТ 1: Линейный поиск (без кэша)
        cout << "\n[1/2] Линейный поиск по " << users.size() << " записям..." << endl;
        cout << "      Выполняется " << ITERATIONS << " запросов (это займёт несколько секунд)..." << endl;
        
        linearSearches = 0;
        long long sum1 = 0;
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; i++) {
            User u = findByNameLinear(testNames[i % 5]);
            sum1 += u.id;
        }
        auto end = high_resolution_clock::now();
        auto linearTime = duration_cast<milliseconds>(end - start).count();
        long long linearSearchCount = linearSearches;
        
        // ТЕСТ 2: Кэширование
        cout << "[2/2] Кэшированный поиск..." << endl;
        cout << "      Выполняется " << ITERATIONS << " запросов..." << endl;
        
        cacheHits = 0;
        cacheMisses = 0;
        linearSearches = 0;
        nameCache.clear();
        long long sum2 = 0;
        
        start = high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; i++) {
            User u = findByNameWithCache(testNames[i % 5], false);
            sum2 += u.id;
        }
        end = high_resolution_clock::now();
        auto cachedTime = duration_cast<milliseconds>(end - start).count();
        
        silentMode = false;
        
        // ВЫВОД РЕЗУЛЬТАТОВ
        cout << "\n========================================" << endl;
        cout << "РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ" << endl;
        cout << "========================================" << endl;
        cout << "Количество записей в БД: " << users.size() << endl;
        cout << "Количество запросов: " << ITERATIONS << endl;
        cout << "Уникальных запросов: 5" << endl;
        cout << "Контрольная сумма ID (линейный): " << sum1 << endl;
        cout << "Контрольная сумма ID (кэш): " << sum2 << endl;
        
        cout << "\n┌─────────────────────────┬──────────────┬────────────────┐" << endl;
        cout << "│        Метод            │   Время(мс)  │  Линейных поисков│" << endl;
        cout << "├─────────────────────────┼──────────────┼────────────────┤" << endl;
        cout << "│ Линейный поиск          │     " << linearTime << " мс       │      " << linearSearchCount << "           │" << endl;
        cout << "│ Кэширование             │     " << cachedTime << " мс       │      " << linearSearches << "            │" << endl;
        cout << "└─────────────────────────┴──────────────┴────────────────┘" << endl;
        
        if (cachedTime > 0 && linearTime > 0) {
            double speedup = (double)linearTime / cachedTime;
            cout << "\n📊 УСКОРЕНИЕ: " << speedup << "x" << endl;
            
            cout << "\n📈 СТАТИСТИКА КЭША:" << endl;
            cout << "   Попаданий: " << cacheHits << endl;
            cout << "   Промахов: " << cacheMisses << endl;
            cout << "   Hit ratio: " << (cacheHits * 100.0 / (cacheHits + cacheMisses)) << "%" << endl;
            
            cout << "\n📈 ТЕОРЕТИЧЕСКАЯ ОЦЕНКА:" << endl;
            cout << "   Без кэша:  " << ITERATIONS << " × " << users.size() 
                 << " = " << (long long)ITERATIONS * users.size() << " сравнений" << endl;
            cout << "   С кэшем:   " << 5 << " × " << users.size() 
                 << " + " << (ITERATIONS - 5) << " = " 
                 << (5LL * users.size() + (ITERATIONS - 5)) << " сравнений" << endl;
            
            double theoreticalSpeedup = (double)((long long)ITERATIONS * users.size()) / (5LL * users.size() + (ITERATIONS - 5));
            cout << "   Теоретическое ускорение: " << theoreticalSpeedup << "x" << endl;
        }
    }
    
    void demonstrateCacheEfficiency() {
        silentMode = false;
        cout << "\n=== ДЕМОНСТРАЦИЯ РАБОТЫ КЭША ===" << endl;
        
        cacheHits = 0;
        cacheMisses = 0;
        linearSearches = 0;
        nameCache.clear();
        
        string queries[] = {"User_1", "User_2", "User_1", "User_3", "User_2", "User_1"};
        
        for (int i = 0; i < 6; i++) {
            cout << "\nЗапрос " << (i+1) << ": \"" << queries[i] << "\"" << endl;
            
            auto start = high_resolution_clock::now();
            User u = findByNameWithCache(queries[i], true);
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start).count();
            
            cout << "  Результат: ID=" << u.id << ", Имя=" << u.name 
                 << ", время=" << duration << " мкс" << endl;
        }
        
        cout << "\n📊 ИТОГО: попаданий=" << cacheHits << ", промахов=" << cacheMisses 
             << ", линейных поисков=" << linearSearches << endl;
    }
    
    void displayStats() {
        cout << "\n=== СТАТИСТИКА ===" << endl;
        cout << "Всего пользователей: " << users.size() << endl;
        cout << "Размер кэша: " << nameCache.size() << endl;
    }
    
    void clearCache() {
        nameCache.clear();
        cacheHits = 0;
        cacheMisses = 0;
        linearSearches = 0;
        cout << "Кэш очищен" << endl;
    }
};

void displayMenu() {
    cout << "\n========================================" << endl;
    cout << "   ОПТИМИЗАЦИЯ ИНФОРМАЦИОННОЙ МОДЕЛИ" << endl;
    cout << "========================================" << endl;
    cout << "1. Демонстрация работы кэша" << endl;
    cout << "2. Сравнение производительности (ГЛАВНЫЙ ТЕСТ)" << endl;
    cout << "3. Показать статистику" << endl;
    cout << "4. Очистить кэш" << endl;
    cout << "5. Выйти" << endl;
    cout << "Выберите действие: ";
}

int main() {
    setlocale(LC_ALL, "Russian");
    
    cout << "========================================" << endl;
    cout << "   ОПТИМИЗАЦИЯ ИНФОРМАЦИОННОЙ МОДЕЛИ" << endl;
    cout << "            (Кэширование)" << endl;
    cout << "========================================" << endl;
    
    InformationModel model;
    int choice;
    
    while (true) {
        displayMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                model.demonstrateCacheEfficiency();
                break;
            case 2:
                model.performanceComparison();
                break;
            case 3:
                model.displayStats();
                break;
            case 4:
                model.clearCache();
                break;
            case 5:
                cout << "Выход..." << endl;
                return 0;
            default:
                cout << "Неверный выбор!" << endl;
                cin.clear();
                cin.ignore(10000, '\n');
                break;
        }
    }
    
    return 0;
}

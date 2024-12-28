#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <memory>
#include <stack>
#include <chrono>

using namespace std;

// Структура для представления узла графа
struct Node {
    double lon, lat; // Долгота и широта узла
    vector<pair<Node*, double >> edges; // Список рёбер, исходящих из узла
};

// Хэш-функция для пар (долгота, широта) для использования в unordered_map
struct pair_hash {
    // Шаблонный оператор для вычисления хэша пары значений
    template <class T1, class T2> 
    size_t operator() (const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first); // Вычисляем хэш первого элемента пары
        auto h2 = hash<T2>{}(p.second); // Вычисляем хэш второго элемента пары
        return h1 ^ (h2 << 1); // Возвращаем комбинированный хэш с использованием побитового XOR и сдвига
    }
};

// Структура для представления графа
struct Graph {
    unordered_map<pair<double, double>, unique_ptr<Node>, pair_hash> nodes; // Хранилище узлов графа

    // Метод для поиска ближайшего узла к заданным координатам (широта, долгота)
    Node* find_closest_node(double lat, double lon) {
        double min_distance = numeric_limits<double>::max(); // Инициализация минимального расстояния
        Node* node_founded = nullptr; // Указатель на найденный узел

        // Проход по всем узлам графа
        for (const auto& node : nodes) {
            // Вычисление евклидова расстояния до текущего узла
            double distance = sqrt(pow(node.second->lat - lat, 2) + pow(node.second->lon - lon, 2));
            if (distance < min_distance) { // Если найденное расстояние меньше минимального
                node_founded = node.second.get(); // Обновление указателя на найденный узел
                min_distance = distance; // Обновление минимального расстояния
            }
        }

        return node_founded; // Возвращение указателя на ближайший узел
    }

    // Метод для добавления нового узла в граф
    void add_node(double lon, double lat) {
        // Проверка, существует ли узел с такими координатами
        if (nodes.find({lat, lon}) == nodes.end()) {
            auto node = make_unique<Node>(); // Создание уникального указателя на новый узел
            node->lon = lon; // Установка долготы узла
            node->lat = lat; // Установка широты узла
            nodes[{lat, lon}] = move(node); // Добавление узла в хранилище графа
        }
    }

    // Метод для добавления ребра между двумя узлами
    void add_edge(double lon1, double lat1, double lon2, double lat2, double distance) {
        auto it1 = nodes.find({lat1, lon1}); // Поиск первого узла по координатам
        auto it2 = nodes.find({lat2, lon2}); // Поиск второго узла по координатам

        if (it1 != nodes.end() && it2 != nodes.end()) { // Если оба узла найдены
            Node* node1 = it1->second.get(); // Получение указателя на первый узел
            Node* node2 = it2->second.get(); // Получение указателя на второй узел
            node1->edges.emplace_back(node2, distance); // Добавление второго узла в список рёбер первого узла
            node2->edges.emplace_back(node1, distance); // Добавление первого узла в список рёбер второго узла (двустороннее ребро)
        }
    }

    // Метод для загрузки графа из файла
    void load_graph_from_file(const string& filename) {
        ifstream file(filename); // Открытие файла для чтения
        string line; // Переменная для хранения текущей строки

        while (getline(file, line)) { // Чтение файла построчно
            stringstream ss(line); // Создание потока из строки
            string node_part, neighbor_part; // Переменные для хранения части строки с узлом и соседями

            getline(ss, node_part, ':'); // Получение части строки до двоеточия (узел)
            double lon1, lat1; 
            sscanf(node_part.c_str(), "%lf,%lf", &lon1, &lat1); // Парсинг координат узла из строки
            add_node(lon1, lat1); // Добавление узла в граф

            while (getline(ss, neighbor_part, ';')) { // Чтение соседей, разделенных точкой с запятой
                double lon2, lat2, distance; 
                sscanf(neighbor_part.c_str(), "%lf,%lf,%lf", &lon2, &lat2, &distance); // Парсинг координат соседа и расстояния из строки
                add_node(lon2, lat2); // Добавление соседа в граф
                add_edge(lon1, lat1, lon2, lat2, distance); // Добавление ребра между узлом и соседом
            }
        }
    }

    // BFS
    // V - узлы, E – рёбра
    vector<Node*> bfs(Node* start, Node* finish) { // сложность O(1), память O(1)
        queue<Node*> q; // сложность O(1), память O(V) Очередь для хранения узлов для обхода
        unordered_map<Node*, Node*> parent; // сложность O(1), память O(V) Хранилище для отслеживания родительских узлов
        unordered_set<Node*> visited; // сложность O(1), память O(V) Множество посещенных узлов
        vector<Node*> path; // сложность O(1), память O(V) Вектор для хранения найденного пути

        q.push(start); // сложность O(1), память O(1) Добавление стартового узла в очередь
        visited.insert(start); // сложность O(1), память O(1) Пометка стартового узла как посещенного
        parent[start] = nullptr; // сложность O(1), память O(1) Установка родителя стартового узла как nullptr

        while (!q.empty()) { // сложность O(V), память O(1) Пока очередь не пуста
            Node* current = q.front(); // сложность O(1), память O(1) Получение текущего узла из очереди
            q.pop(); // сложность O(1), память O(1) Удаление текущего узла из очереди

            if (current == finish) { // сложность O(1), память O(1) Если достигнут конечный узел
                while (current) { // сложность O(V), память O(1)
                    path.push_back(current); // сложность O(1), память O(1) Добавление текущего узла в путь
                    current = parent[current]; // сложность O(1), память O(1) Переход к родительскому узлу
                }
                reverse(path.begin(), path.end()); // сложность O(V), память O(1) Обратный порядок пути (от конечного к стартовому)
                return path; // сложность O(1), память O(1) Возвращение найденного пути
            }

            for (auto [neighbor, _] : current->edges) { // сложность O(E), память O(1) Проход по всем соседям текущего узла
                if (visited.find(neighbor) == visited.end()) { // сложность O(1), память O(1) Если сосед еще не посещен
                    visited.insert(neighbor); // сложность O(1), память O(1) Пометка соседа как посещенного
                    parent[neighbor] = current; // сложность O(1), память O(1) Установка текущего узла как родителя соседа
                    q.push(neighbor); // сложность O(1), память O(1) Добавление соседа в очередь для дальнейшего обхода
                }
            }
        }
        return path; // сложность O(1), память O(1) Возвращение пустого пути, если конечный узел не найден
    }
    // Общая временная сложность - O(V + E), общее использование памяти - O(V)

    // DFS
    // V - узлы, E – рёбра
    vector<Node*> dfs(Node* start, Node* finish) { // сложность O(1), память O(1)
        stack<Node*> s; // сложность O(1), память O(V) Создаем стек для хранения узлов во время обхода графа
        unordered_map<Node*, Node*> parent; // сложность O(1), память O(V) Хранилище для отслеживания родительских узлов (для восстановления пути)
        unordered_set<Node*> visited; // сложность O(1), память O(V) Множество для хранения посещенных узлов
        vector<Node*> path; // сложность O(1), память O(V) Вектор для хранения найденного пути

        s.push(start); // сложность O(1), память O(1) Добавляем стартовый узел в стек для начала обхода
        visited.insert(start); // сложность O(1), память O(1) Помечаем стартовый узел как посещенный
        parent[start] = nullptr; // сложность O(1), память O(1) Устанавливаем родителя стартового узла как nullptr (нет родителя)

        while (!s.empty()) { // сложность O(V), память O(1) Пока стек не пустой
            Node* current = s.top(); // сложность O(1), память O(1) Получаем текущий узел из вершины стека
            s.pop(); // сложность O(1), память O(1) Удаляем текущий узел из стека

            if (current == finish) { // сложность O(1), память O(1) Если достигнут конечный узел
                while (current) { // сложность O(V), память O(1) Восстанавливаем путь от конечного до стартового узла
                    path.push_back(current); // сложность O(1), память O(1) Добавляем текущий узел в путь
                    current = parent[current]; // сложность O(1), память O(1) Переходим к родительскому узлу
                }
                reverse(path.begin(), path.end()); // сложность O(V), память O(1) Оборачиваем путь, чтобы он был от стартового до конечного узла
                return path; // сложность O(1), память O(1) Возвращаем найденный путь
            }

            for (auto [neighbor, _] : current->edges) { // сложность O(E), память O(1) Проходим по всем соседям текущего узла
                if (visited.find(neighbor) == visited.end()) { // сложность O(1), память O(1) Если сосед еще не посещен
                    visited.insert(neighbor); // сложность O(1), память O(1) Помечаем соседа как посещенного
                    parent[neighbor] = current; // сложность O(1), память O(1) Устанавливаем текущий узел как родителя соседа
                    s.push(neighbor); // сложность O(1), память O(1) Добавляем соседа в стек для дальнейшего обхода
                }
            }
        }
        return path; // сложность O(1), память O(1) Возвращаем пустой путь, если конечный узел не найден
    }
    // Общая временная сложность - O(V + E), общее использование памяти - O(V)

    // Dijkstra
    // V - узлы, E – рёбра
    vector<Node*> dijkstra(Node* start, Node* finish) { // сложность O(1), память O(1)
        priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> pq; // сложность (O(1)), память (O(V)) Создаем приоритетную очередь для хранения узлов с их расстояниями
        unordered_map<Node*, double> distances; // сложность O(1), память O(V) Хранилище для расстояний от стартового узла до каждого узла
        unordered_map<Node*, Node*> parent; // сложность (O(1)), память (O(V)) Хранилище для отслеживания родительских узлов

        for (auto& node : nodes) { // сложность O(V), память O(1)
            distances[node.second.get()] = numeric_limits<double>::infinity(); // Инициализируем расстояния до всех узлов как бесконечность
        }

        distances[start] = 0; // сложность O(1), память O(1) Расстояние до стартового узла равно 0
        pq.push({0, start}); // сложность O(log V), память O(1) Добавляем стартовый узел в очередь с расстоянием 0
        parent[start] = nullptr; // сложность O(1), память O(1) Устанавливаем родителя стартового узла как nullptr

        while (!pq.empty()) { // сложность O(V) память O(1) Пока очередь не пуста
            auto [current_dist, current] = pq.top(); // сложность O(1), память O(1) Получаем узел с наименьшим расстоянием из очереди
            pq.pop(); // сложность O(log V), память O(1) Удаляем этот узел из очереди

            if (current_dist > distances[current]) continue; // сложность O(1), память O(1) Если текущее расстояние больше уже известного, пропускаем этот узел

            if (current == finish){ // сложность O(1), память O(1) Если достигнут конечный узел
                vector<Node*> path; // сложность O(1), память O(V) Вектор для хранения найденного пути
                while (current) { // сложность O(V), память O(1) Восстанавливаем путь от конечного до стартового узла
                    path.push_back(current); // сложность O(1), память O(1) Добавляем текущий узел в путь
                    current = parent[current]; // сложность O(1), память O(1) Переходим к родительскому узлу
                }
                reverse(path.begin(), path.end()); // сложность O(V), память O(1) Оборачиваем путь, чтобы он был от стартового до конечного узла
                return path; // сложность O(1), память O(1) Возвращаем найденный путь
            };

            for (auto [neighbor, edge_dist] : current->edges) { // сложность O(E), память O(1) Проходим по всем соседям текущего узла
                double new_dist = current_dist + edge_dist; // сложность O(1), память O(1) Вычисляем новое расстояние до соседа

                if (new_dist < distances[neighbor]) { // сложность O(1), память O(1) Если новое расстояние меньше уже известного расстояния до соседа
                    distances[neighbor] = new_dist; // сложность O(1), память O(1) Обновляем расстояние до соседа
                    pq.push({new_dist, neighbor}); // сложность O(log V), память O(1) Добавляем соседа в очередь с обновленным расстоянием
                    parent[neighbor] = current; // сложность O(1), память O(1 Устанавливаем текущий узел как родителя соседа
                }
            }
        }
        
        return {}; // Возвращаем пустой вектор, если путь не найден
    }
    // Общая временная сложность - O((V + E)log V), общее использование памяти - O(V)

    // Вычисление расстояния между двумя узлами
    double heuristic(Node* a, Node* b) { // сложность O(1), память O(1)
        return sqrt(pow(a->lat - b->lat, 2) + pow(a->lon - b->lon, 2)); // сложность O(1), память O(1)
    }

    // A*
    vector<Node *> a_star(Node* start, Node* finish) { // сложность O(1), память O(1)
        priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> open_set; // сложность O(1), память O(V) Приоритетная очередь для хранения открытых узлов с их оценками f(n)
        unordered_map<Node*, double> g_score; // сложность O(1), память O(V) Хранилище для g(n) - стоимости пути от стартового узла до каждого узла
        unordered_map<Node*, double> f_score; // сложность O(1), память O(V) Хранилище для f(n) - полной стоимости пути через узел (g(n) + h(n))
        unordered_map<Node*, Node*> came_from; // сложность O(1), память O(V) Хранилище для отслеживания родительских узлов

        // Инициализируем g(n) и f(n) для всех узлов как бесконечность
        for (const auto& node : nodes) { // сложность O(V), память O(1)
            g_score[node.second.get()] = numeric_limits<double>::infinity(); // сложность O(1), память O(1)
            f_score[node.second.get()] = numeric_limits<double>::infinity(); // сложность O(1), память O(1)
        }

        g_score[start] = 0; // сложность O(1), память O(1) Стоимость пути от стартового узла до самого себя равна 0
        f_score[start] = heuristic(start, finish); // сложность O(1), память O(1) Вычисляем начальное значение f(n) для стартового узла
        open_set.push({f_score[start], start}); // сложность O(log V), память O(1) Добавляем стартовый узел в открытый список с его оценкой f(n)
        came_from[start] = nullptr; // сложность O(1), память O(1) Устанавливаем родителя стартового узла как nullptr

        while (!open_set.empty()) { // сложность O(V), память O(1) Пока открытый список не пустой
            Node* current = open_set.top().second; // сложность O(1), память O(1) Получаем узел с наименьшей оценкой f(n)
            open_set.pop(); // сложность O(log V), память O(1) Удаляем этот узел из открытого списка

            if (current == finish) { // сложность O(1), память O(1) Если достигнут конечный узел
                vector<Node*> path; // сложность O(1), память O(V) Вектор для хранения найденного пути
                while (current){ // O(V), память O(1) Восстанавливаем путь от конечного до стартового узла
                    path.push_back(current); // сложность O(1), память O(V) Добавляем текущий узел в путь
                    current = came_from[current]; // сложность O(1), память O(1) Переходим к родительскому узлу
                }
                reverse(path.begin(), path.end()); // сложность O(V), память O(1) Разворачиваем путь, чтобы он был от стартового до конечного узла
                return path; // сложность O(1), память O(1) Возвращаем найденный путь
            }

            for (auto [neighbor, weight] : current->edges) { // сложность O(E), память O(1) Проходим по всем соседям текущего узла
                double tentative_g_score = g_score[current] + weight; // Вычисляем предполагаемую стоимость пути до соседа через текущий узел

                if (tentative_g_score < g_score[neighbor]) { // сложность O(1), память O(1) Если новое предполагаемое значение g(n) меньше уже известного
                    came_from[neighbor] = current; // сложность O(1), память O(1) Устанавливаем текущий узел как родителя соседа
                    g_score[neighbor] = tentative_g_score; // сложность O(1), память O(1) Обновляем g(n) для соседа
                    f_score[neighbor] = g_score[neighbor] + heuristic(neighbor, finish); // сложность O(1), память O(1) Обновляем f(n) для соседа
                    open_set.push({f_score[neighbor], neighbor}); // сложность O(log V), память O(1) Добавляем соседа в открытый список с обновленным значением f(n)
                }
            }
        }
        return {}; // сложность O(1), память O(1) Возвращаем пустой вектор, если путь не найден
    }
    // Общая временная сложность - O((V + E)log V), общее использование памяти - O(V)

    // Вывод узлов
    void print_path(vector<Node*>& path) {
        for (Node* n : path) {
            cout << "(" << n->lon << "," << n->lat << ") ";
        }
        cout << endl;
    }

    // Подсчёт времени BFS
    void time_bfs(Node* start_node, Node* finish_node) {
        auto start_time = chrono::high_resolution_clock::now();
        vector<Node*> path = bfs(start_node, finish_node);
        auto end_time = chrono::high_resolution_clock::now();
        // cout << "BFS: ";
        // print_path(path);
        cout << "Время BFS: " << chrono::duration<double>(end_time - start_time).count() << " сек" << endl;
    }

    // Подсчёт времени DFS
    void time_dfs(Node* start_node, Node* finish_node) {
        auto start_time = chrono::high_resolution_clock::now();
        vector<Node*> path = dfs(start_node, finish_node);
        auto end_time = chrono::high_resolution_clock::now();
        // cout << "DFS: ";
        // print_path(path);
        cout << "Время DFS: " << chrono::duration<double>(end_time - start_time).count() << " сек" << endl;
    }

    // Подсчёт времени Dijkstra
    void time_dijkstra(Node* start_node, Node* finish_node) {
        auto start_time = chrono::high_resolution_clock::now();
        vector<Node*> path = dijkstra(start_node, finish_node);
        auto end_time = chrono::high_resolution_clock::now();
        // cout << "Dijkstra: ";
        // print_path(path);
        cout << "Время Dijkstra: " << chrono::duration<double>(end_time - start_time).count() << " сек" << endl;
    }

    // Подсчёт времени A*
    void time_a_star(Node* start_node, Node* finish_node) {
        auto start_time = chrono::high_resolution_clock::now();
        vector<Node*> path = a_star(start_node, finish_node);
        auto end_time = chrono::high_resolution_clock::now();
        // cout << "A*: ";
        // print_path(path);
        cout << "Время A*: " << chrono::duration<double>(end_time - start_time).count() << " сек" << endl;
    }
};


void run_spb_graph(){
    Graph graph; // Создаем объект графа
    graph.load_graph_from_file("spb_graph.txt");

    double start_lon = 30.308307, start_lat = 59.867813; // Координаты начальной точки (общежитие)
    double finish_lon = 30.337795, finish_lat = 59.926835; // Координаты конечной точки (корпус на Ломоносова)

    Node* start_node = graph.find_closest_node(start_lat, start_lon); // Находим ближайший узел к начальной точке
    Node* finish_node = graph.find_closest_node(finish_lat, finish_lon); // Находим ближайший узел к конечной точке

    // Запускаем алгоритмы поиска, измеряем время выполнения и выводим результаты
    graph.time_bfs(start_node, finish_node);
    cout << endl;

    graph.time_dfs(start_node, finish_node);
    cout << endl;

    graph.time_dijkstra(start_node, finish_node);
    cout << endl;

    graph.time_a_star(start_node, finish_node);
    cout << endl;
};

// Тестирование на первом графе
void run_test_1(double start_lat, double start_lon, double finish_lat, double finish_lon){
    Graph graph;
    graph.load_graph_from_file("test1.txt");

    Node* start_node = graph.find_closest_node(start_lat, start_lon);
    Node* finish_node = graph.find_closest_node(finish_lat, finish_lon);

    graph.time_bfs(start_node, finish_node);
    cout << endl;

    graph.time_dfs(start_node, finish_node);
    cout << endl;

    graph.time_dijkstra(start_node, finish_node);
    cout << endl;

    graph.time_a_star(start_node, finish_node);
    cout << endl;
};

// Тестирование на втором графе
void run_test_2(double start_lat, double start_lon, double finish_lat, double finish_lon){
    Graph graph;
    graph.load_graph_from_file("test2.txt");

    Node* start_node = graph.find_closest_node(start_lat, start_lon);
    Node* finish_node = graph.find_closest_node(finish_lat, finish_lon);

    graph.time_bfs(start_node, finish_node);
    cout << endl;

    graph.time_dfs(start_node, finish_node);
    cout << endl;

    graph.time_dijkstra(start_node, finish_node);
    cout << endl;

    graph.time_a_star(start_node, finish_node);
    cout << endl;
};

int main() {

    run_spb_graph();

    // cout << " Тест 1: " << endl;// 0 -> 1
    // run_test_1(0,0,1,1);
    // cout << " Тест 2: " << endl;// 0 -> 2
    // run_test_1(0,0,2,2);
    // cout << " Тест 3: " << endl;// 1 -> 3
    // run_test_1(1,1,3,3);
    // cout << " Тест 4: " << endl;// 0 -> 3
    // run_test_1(0,0,3,3);
    // cout << " Тест 5: " << endl;// 2 -> 4
    // run_test_1(2,2,4,4);
    // cout << " Тест 6: " << endl;// 1 -> 2
    // run_test_1(1,1,2,2);
    // cout << " Тест 7: " << endl;// 3 -> 2
    // run_test_1(3,3,2,2);

    // cout << " Тест 1: " << endl;// 0 -> 1
    // run_test_2(0,0,1,1);
    // cout << " Тест 2: " << endl;// 0 -> 2
    // run_test_2(0,0,2,2);
    // cout << " Тест 3: " << endl;// 1 -> 3
    // run_test_2(1,1,3,3);
    // cout << " Тест 4: " << endl;// 0 -> 3
    // run_test_2(0,0,3,3);
    // cout << " Тест 5: " << endl;// 2 -> 4
    // run_test_2(2,2,4,4);
    // cout << " Тест 6: " << endl;// 1 -> 2
    // run_test_2(1,1,2,2);
    // cout << " Тест 7: " << endl;// 3 -> 2
    // run_test_2(3,3,2,2);

    return 0;
}
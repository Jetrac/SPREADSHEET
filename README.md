# SPREADSHEET

Электронная таблица - учебный проект Яндекс Практикума на C++, который предоставляет функциональные возможности для управления таблицей ячеек. Ячейки могут содержать обычный текст или формулу, ссылающуюся на другие ячейки для выполнения арифметических вычислений.

## Использование

Проект предоставляет следующие функциональные возможности:

1. Управление ячейками:
   - `CellInterface`: Может содержать обычный текст или формулы. Поддерживает установку, получение и очистку значений ячеек.
   - `CreateSheet()`: Создает новую таблицу, содержащую ячейки.
Пример:
```cpp
auto sheet = CreateSheet();
sheet->SetCell("A1"_pos, "Hello");
sheet->SetCell("A2"_pos, "=1+2");
sheet->ClearCell("A1"_pos);
```

2. Вычисление формулы:
   - `ParseFormula()`: Анализирует формульное выражение и возвращает указатель на формулу.
   - `Formula::Evaluate()`: Вычисляет формулу и возвращает результат.
Пример:
```cpp
auto sheet = CreateSheet();
sheet->SetCell("A1"_pos, "2");
sheet->SetCell("A2"_pos, "3");
auto result = std::get<double>(ParseFormula("A1+A2")->Evaluate(*sheet));
std::cout << result; // Output: 5
```
    
3. Форматирование выражений:
   - `ParseFormula()`: Анализирует формульное выражение и возвращает указатель на формулу.
   - `Formula::GetExpression()`: Форматирует обработанное формульное выражение для отображения.
Пример:
```cpp
std::string expr = "2 + 2*2";
auto formattedExpr = ParseFormula(expr)->GetExpression();
std::cout << formattedExpr; // Output: 2*2+2
```
4. Обработка ошибок:
   - Проект поддерживает различные категории ошибок, такие как "FormulaError::Category::Value" и "FormulaError::Category::!Arithm", которые указывают на ошибки при вычислении формулы.
Пример:
```cpp
auto sheet = CreateSheet();
sheet->SetCell("A1"_pos, "=1/0");
auto result = sheet->GetCell("A1"_pos)->GetValue();
if (std::holds_alternative<FormulaError>(result)) {
    std::cout << "Error: " << std::get<FormulaError>(result).ToString();
}
```

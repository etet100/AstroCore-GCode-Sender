# Analiza systemu konwerterów - Pipeline vs. Pull mode

## Architektura

### Klasy i interfejsy
```
ConverterInterface (abstract)
├── Pipeline (ConverterInterface + Converter)
└── SingleConverter (ConverterInterface wrapper)

Converter (abstract base)
├── FeedRateConverter
├── CoordinateOffsetConverter
├── ApplyHeightmap
└── ...inne
```

---

## Tryb 1: convertNext() - Pull Mode

### Jak działa Pipeline.convertNext(count)
```cpp
int Pipeline::convertNext(int count) {
    int endIndex = qMin(m_currentIndex + count, m_gcode->count());

    for (int i = m_currentIndex; i < endIndex; ++i) {
        processLine(i);  // ← Przetwarza linię i
        m_currentIndex++;
    }
}

bool Pipeline::processLine(int index) {
    GCodeItem &item = (*m_gcode)[index];

    // Wszystkie convertery po kolei
    for (Converter *converter : m_converters) {
        converter->convertLine(item, m_gcode, index, m_parser);
    }
}
```

### ⚠️ PROBLEM: Wstawiane linie nie są przetwarzane

**Scenariusz:**
```
Initial GCode: [0, 1, 2, 3, 4, 5]
Current index: 0
Request: convertNext(3)  // Przetwórz 3 linie
```

**Krok 1:** Przetwarza linię 0
- ApplyHeightmap segmentuje linię 0 na 4 segmenty
- Modyfikuje linię 0
- **Wstawia linie 1, 2, 3** przez `gcode->insert(1, newItem)`
```
GCode po: [0_mod, NEW_1, NEW_2, NEW_3, 1, 2, 3, 4, 5]
                                    ↑ oryginalna linia 1
m_currentIndex = 0 → 1
```

**Krok 2:** Przetwarza linię 1 (index=1)
- **To jest NEW_1, nie oryginalna linia!**
- NEW_1 nie została przetworzona przez wcześniejsze convertery
- Jeśli pipeline ma [Converter1, ApplyHeightmap, Converter2]:
  - NEW_1 przejdzie tylko przez Converter2
  - **Pomija Converter1!**

**Krok 3:** Przetwarza linię 2 (index=2)
- To jest NEW_2

**Koniec:** m_currentIndex = 3
- **Oryginalne linie 1, 2, 3 są teraz na indeksach 4, 5, 6**
- **Nie zostaną przetworzone w tym wywołaniu!**

### 🔴 Wnioski dla Pull Mode
- **Wstawione linie pomijają wcześniejsze convertery w pipeline**
- **Następne wywołania convertNext() przetworzą je jak zwykłe linie**
- **Oryginalne linie zostają przesunięte i mogą być pominięte**

---

## Tryb 2: convertAll() - Full Conversion

### Jak działa Pipeline.convertAll()
```cpp
GCode* Pipeline::convertAll() {
    GCode *result = new GCode();
    *result << *m_gcode;  // Kopiuje wszystkie linie

    for (int i = 0; i < result->count(); ++i) {  // ← count() może rosnąć!
        GCodeItem &item = (*result)[i];

        for (Converter *converter : m_converters) {
            converter->convertLine(item, result, i, m_parser);
        }
    }

    return result;
}
```

### ⚠️ PROBLEM: Wstawione linie pomijają wcześniejsze convertery

**Scenariusz:**
```
Pipeline: [Converter1, ApplyHeightmap, Converter2]
Initial result: [0, 1, 2, 3, 4]
```

**i=0:** Przetwarza linię 0
- Converter1 → modyfikuje linię 0
- ApplyHeightmap → wstawia [NEW_1, NEW_2] na index 1
- Converter2 → przetwarza zmodyfikowaną linię 0
```
result: [0_mod, NEW_1, NEW_2, 1, 2, 3, 4]
result->count() = 7
```

**i=1:** Przetwarza linię 1 (NEW_1)
- Converter1 → przetwarza NEW_1
- ApplyHeightmap → może wstawić więcej linii
- Converter2 → przetwarza

**i=2:** Przetwarza linię 2 (NEW_2)
- Wszystkie convertery

**i=3:** Przetwarza linię 3 (oryginalna linia 1)
- Wszystkie convertery
- **ALE ApplyHeightmap znowu ją podzieli na segmenty!**
- Wstawia kolejne linie...

### 🟡 Wnioski dla convertAll()
- **Wstawione linie PRZECHODZĄ przez wszystkie convertery**
- **ALE kolejność jest zła: najpierw Converter1, potem ApplyHeightmap**
- **Parser może się pogubić - nowe linie nie aktualizują stanu parsera**

---

## Szczegółowa analiza ApplyHeightmap

### Jak wstawia linie
```cpp
bool ApplyHeightmap::convertLine(...) {
    // Segmentuje linię na N punktów
    QList<QVector3D> points;  // np. 5 punktów = 4 segmenty

    // Modyfikuje current line na pierwszy segment
    item.line = generateGCodeLine(points[0], points[1], ...);

    // Wstawia pozostałe segmenty
    for (int i = 1; i < points.size() - 1; i++) {
        GCodeItem newItem;
        newItem.line = generateGCodeLine(points[i], points[i + 1], ...);

        gcode->insert(currentIndex + i, newItem);  // ← WSTAWIA
    }
}
```

### Problem z indeksami
```
Wstawia na: currentIndex + 1, currentIndex + 2, currentIndex + 3...
```
- W Pull Mode: następna iteracja przetwórz currentIndex + 1, ale to wstawiona linia!
- W convertAll(): pętla for dojdzie do tych linii i przetworzy je ponownie

---

## Możliwe rozwiązania

### ❌ ROZWIĄZANIE 1: Poprawka w Pipeline (skomplikowane)

Modyfikacja `convertNext()`:
```cpp
int Pipeline::convertNext(int count) {
    int processed = 0;

    while (processed < count && m_currentIndex < m_gcode->count()) {
        int originalIndex = m_currentIndex;
        int originalCount = m_gcode->count();

        processLine(m_currentIndex);

        int insertedLines = m_gcode->count() - originalCount;
        m_currentIndex += insertedLines + 1;  // Pomiń wstawione
        processed++;
    }
}
```

**Problemy:**
- Wstawione linie nigdy nie przejdą przez pipeline
- Wymaga oznaczania linii jako "już przetworzone"

### ❌ ROZWIĄZANIE 2: Dwufazowa konwersja

```cpp
for (Converter *converter : m_converters) {
    for (int i = 0; i < gcode->count(); ++i) {
        converter->convertLine(...);
    }
}
```

**Problemy:**
- Parser traci stan między fazami
- Bardzo wolne dla wielu konwerterów

### ✅ ROZWIĄZANIE 3: ApplyHeightmap jako standalone (REKOMENDOWANE)

ApplyHeightmap **NIE POWINIEN** być w Pipeline. Powinien:

1. **Implementować własny convertAll()** bez dziedziczenia po Converter
2. **Przetwarzać cały GCode za jednym razem**
3. **Kontrolować własny parser i indeksy**

```cpp
class ApplyHeightmap : public ConverterInterface  // NIE Converter!
{
public:
    GCode* convertAll() override {
        GCode* result = new GCode();
        int sourceIndex = 0;

        while (sourceIndex < m_gcode->count()) {
            GCodeItem item = m_gcode->at(sourceIndex);

            if (item.isMovement) {
                QList<GCodeItem> segments = segmentLine(item);
                for (const auto& seg : segments) {
                    *result << seg;
                }
            } else {
                *result << item;
            }

            sourceIndex++;
        }

        return result;
    }
};
```

### ✅ ROZWIĄZANIE 4: Batch buffering w Converter

Zamiast wstawiać bezpośrednio, zwracaj dodatkowe linie:

```cpp
class Converter {
    virtual bool convertLine(GCodeItem &item, ...) = 0;
    virtual QList<GCodeItem> getBufferedLines() { return {}; }
};
```

Pipeline pobiera buffered lines i wstawia je PO przetworzeniu przez wszystkie convertery.

---

## Rekomendacje

### Dla ApplyHeightmap - OPCJA A: Standalone
```cpp
// Nie używaj w Pipeline!
ApplyHeightmap heightmapConverter(heightmap, 1.0);
heightmapConverter.setGCode(originalGCode);
GCode* result = heightmapConverter.convertAll();
```

### Dla ApplyHeightmap - OPCJA B: Ostatni w Pipeline
```cpp
// Umieść ApplyHeightmap jako OSTATNI converter
Pipeline pipeline;
pipeline << new FeedRateConverter(1.5);
pipeline << new CoordinateOffsetConverter(10, 10, 0);
pipeline << new ApplyHeightmap(heightmap, 1.0);  // OSTATNI!

GCode* result = pipeline.convertAll();
// Wstawione linie nie będą przetworzone przez wcześniejsze convertery
// ALE to akceptowalne - heightmap jest ostatnią modyfikacją
```

### Dla Pipeline - Pull Mode
**NIE UŻYWAJ z converterami które wstawiają linie!**

Pull mode działa tylko dla konwerterów które:
- Modyfikują linie in-place (jak FeedRateConverter)
- Nie wstawiają nowych linii
- Nie usuwają linii

---

## Testy które trzeba wykonać

### Test 1: Pull mode z prostymi converterami
```cpp
Pipeline pipeline;
pipeline << new FeedRateConverter(1.5);
pipeline.setGCode(gcode);

while (pipeline.hasMore()) {
    pipeline.convertNext(10);
}
// Oczekiwane: wszystkie linie przetworzone
```

### Test 2: convertAll() z ApplyHeightmap
```cpp
Pipeline pipeline;
pipeline << new ApplyHeightmap(heightmap, 1.0);
pipeline.setGCode(gcode);

GCode* result = pipeline.convertAll();
// Sprawdź: czy liczba linii się zgadza?
// Czy wszystkie segmenty mają poprawny Z?
```

### Test 3: Mieszany pipeline
```cpp
Pipeline pipeline;
pipeline << new FeedRateConverter(1.5);  // Przed
pipeline << new ApplyHeightmap(heightmap, 1.0);
pipeline << new CommentAdder();  // Po

GCode* result = pipeline.convertAll();
// Sprawdź: czy FeedRateConverter zmienił feedrate w ORYGINALNYCH liniach?
// Czy CommentAdder dodał komentarze do NOWYCH linii od ApplyHeightmap?
```

---

## Wnioski końcowe

1. **Pull mode (convertNext) NIE DZIAŁA z ApplyHeightmap** - wstawione linie pomijają pipeline

2. **convertAll() CZĘŚCIOWO DZIAŁA** - ale wstawione linie nie przechodzą przez wcześniejsze convertery

3. **Rekomendacja:** ApplyHeightmap powinien być **standalone** lub **ostatni w pipeline**

4. **Pipeline wymaga redesignu** jeśli ma obsługiwać convertery wstawiające linie w pull mode

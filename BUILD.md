# Static Sort - Instructions de Build

## Compilation avec CMake

### Build basique

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Exécuter l'exemple principal

```bash
./build/example_main
```

### Build avec les benchmarks

```bash
mkdir build
cd build
cmake -DBUILD_BENCHMARKS=ON ..
cmake --build .
```

Cela créera les exécutables suivants :
- `example_main` (exemple par défaut)
- `bench_6` (benchmark avec 6 éléments)
- `bench_10` (benchmark avec 10 éléments)
- `bench_n` (benchmark avec N éléments)
- `bench_pair_sort` (benchmark de tri par paires)

### Installation (optionnelle)

```bash
mkdir build
cd build
cmake -DINSTALL_STATIC_SORT=ON ..
cmake --build .
sudo cmake --install .
```

## Configuration

Le projet utilise **C++20** et nécessite un compilateur compatible :
- GCC 10+
- Clang 10+
- MSVC 2019+
- AppleClang 12+

Les optimisations sont automatiquement activées (-O3 pour GCC/Clang, /O2 pour MSVC).

## Utilisation en tant que header-only

Vous pouvez aussi simplement copier `include/static_sort.h` dans votre projet et l'inclure directement sans utiliser CMake.


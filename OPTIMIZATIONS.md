# Optimisations appliquées à StaticSort

## Résumé des améliorations

### 1. Swap branchless optimisé
- Pour les types triviaux (int, double, float, pointeurs), utilisation de min/max conditionnels
- Le compilateur peut générer des instructions CMOV (conditional move) au lieu de branches
- Réduit les branch mispredictions

### 2. Spécialisations avec réseaux de tri optimaux
Les spécialisations suivantes ont été ajoutées avec le nombre optimal de comparaisons :

- **2 éléments** : 1 comparaison
- **3 éléments** : 3 comparaisons (réseau optimal)
- **4 éléments** : 5 comparaisons (réseau optimal)
- **5 éléments** : 9 comparaisons (réseau optimal)
- **6 éléments** : 12 comparaisons (réseau optimal)
- **7 éléments** : 16 comparaisons (réseau optimal)
- **8 éléments** : 19 comparaisons (réseau optimal)

### 3. Attributs de compilation
- `[[gnu::always_inline]]` sur les fonctions de swap et de tri
- Force l'inlining des fonctions critiques pour éliminer l'overhead d'appel

### 4. Optimisations move semantics
- Utilisation systématique de `std::move` pour les types non-triviaux
- Évite les copies inutiles

## Résultats des benchmarks

### Données aléatoires (Worst case)

```
Taille  | std::sort | StaticSort | Amélioration
--------|-----------|------------|-------------
   2    |  18.7 ns  |  18.2 ns   |   2.7%
   3    |  26.5 ns  |  24.6 ns   |   7.2%
   4    |  36.7 ns  |  34.1 ns   |   7.1%
   5    |  45.5 ns  |  41.5 ns   |   8.8%
   6    |  52.7 ns  |  48.6 ns   |   7.8%
   7    |  61.7 ns  |  54.5 ns   |  11.7%
   8    |  72.0 ns  |  66.2 ns   |   8.1%
```

### StaticTimSort - Best case (données déjà triées)

```
Taille  | StaticTimSort Random | StaticTimSort Sorted | Amélioration
--------|---------------------|----------------------|-------------
   8    |      66.3 ns        |       1.93 ns        |   34x plus rapide
  16    |      59.7 ns        |       2.46 ns        |   24x plus rapide
```

### StaticTimSort - Données inversées

```
Taille  | StaticTimSort Random | StaticTimSort Reversed | Amélioration
--------|---------------------|------------------------|-------------
   8    |      66.3 ns        |       4.31 ns          |   15x plus rapide
  16    |      59.7 ns        |       3.93 ns          |   15x plus rapide
```

## Recommandations d'utilisation

1. **StaticSort** : 
   - Meilleur pour données vraiment aléatoires
   - Jusqu'à 12% plus rapide que std::sort pour petits tableaux (7-8 éléments)
   - Performances constantes quelle que soit la distribution

2. **StaticTimSort** :
   - Excellente pour données partiellement triées
   - Jusqu'à 34x plus rapide sur données déjà triées
   - Détection automatique des runs (séquences triées)
   - Overhead minimal (< 1ns) sur données aléatoires

3. **Seuil de rentabilité** :
   - Pour N ≤ 16 éléments : StaticSort/StaticTimSort
   - Pour N > 16 éléments : std::sort devient plus efficace

## Techniques utilisées

### Branchless swap
```cpp
// Version branchless pour types triviaux
T min_val = (v1 < v0) ? v1 : v0;
T max_val = (v1 < v0) ? v0 : v1;
v0 = min_val;
v1 = max_val;
```

### Réseaux de tri optimaux
Les réseaux sont générés à la compilation via template metaprogramming.
Exemple pour 4 éléments :
```cpp
if (lt(b, a)) std::swap(a, b);  // Compare pairs
if (lt(d, c)) std::swap(c, d);
if (lt(c, a)) std::swap(a, c);  // Merge
if (lt(d, b)) std::swap(b, d);
if (lt(c, b)) std::swap(b, c);  // Final compare
```

## Optimisations futures possibles

1. **SIMD** : Utiliser AVX2/AVX-512 pour tri parallèle de petits tableaux
2. **Spécialisations par type** : Int32, Int64, Float, Double avec instructions dédiées
3. **Prefetching** : Pour tableaux > 8 éléments
4. **Réseaux adaptatifs** : Sélection dynamique du réseau selon distribution détectée


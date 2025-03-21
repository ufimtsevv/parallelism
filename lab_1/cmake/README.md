# Сборка с CMake

Для `Float`:

```
mkdir build
cd build
cmake -DUSE_FLOAT=ON ..
cmake --build .
```

Для `Double`:

```
mkdir build
cd build
cmake -DUSE_FLOAT=OFF ..
cmake --build .
```

### Вывод

Для `Float`:

```
type: float
time: 0.492309 sec
sum: -0.0277862
```

Для `Double`:

```
type: double
time: 0.497169 sec
sum: 4.89582e-11
```

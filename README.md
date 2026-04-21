# Dokumentacja silnika dBengine

Poniżej prezentowane są funkcje oferowane przez silnik. dBengine wspiera skryptowanie funkcjonalności gameplay'owych z pomocą języka Lua.

Silnik został podzielony na <b>3 kluczowe części</b>:
- <b>`dBengine Core`</b> - funkcje silnika, które nie zostały przypisane do pozostałych kategorii kluczowych;
- <b>`dBrender`</b> - silnik odpowiadający za inicjalizację instancjonowania, zarządzanie instancjami oraz sam rendering;
- <b>`dBphysics`</b> - silnik fizyki. Odpowiada za obliczanie i rozstrzyganie kolizji w scenie;

Natomiast wszelkie ustawienia silnika (takie jak np. VSync) przechowywane są przez klasę `EngineSettings`.

## Debug

>[!TIP]
>Silnik posiada profiler, który zapisuje logi wysłane na konsolę do plików `.log`. Znajdują się w one w folderze `logs`.

### Użycie


Aby odwołać się do obiektu `EngineDebug` poza obiektem `dBengine` (`dBengine` posiada odwołanie pod zmienną `debug`) należy wywołać instancję korzystając z `EngineDebug::GetInstance()`.

>[!NOTE]
>`EngineDebug::GetInstance().PrintInfo(message)` - wypisuje wiadomość informacyjną o wiadomości `message`.

>[!IMPORTANT]
>`EngineDebug::GetInstance().PrintDebug(message)` - wypisuje wiadomość debugową o wiadomości `message`.

>[!CAUTION]
>`EngineDebug::GetInstance().PrintError(message)` - wypisuje błąd o wiadomości `message`.

>[!WARNING]
>`EngineDebug::GetInstance().PrintWarning(message)` - wypisuje ostrzeżenie o wiadomości `message`.

## Skrypty

Template skryptu .lua

```
function onReady()
  -- Funkcja wykonywana raz na starcie skryptu, gdy obiekt zawierający go
  -- wejdzie do sceny.
end

function onUpdate(deltaTime)
  -- Funkcja wywoływana jest co klatkę silnika,
  -- dBengine przekazuje czas od ostatniej klatki
  -- jako argument funkcji.
end

function onMouseMotion(offsetX, offsetY)
  -- Funkcja wywoływana jest, gdy zarejestrowany zostanie
  -- ruch myszką. Silnik przekazuje jako argumenty współczynnik
  -- ruchu myszy.
end
```

### Zmienne eksportowane

Silnik skryptów umożliwia na eksportowanie zmiennych do silnika, aby można było je edytować w inspektorze. Zmiany zmiennych eksportowanych są od razu widoczne w uruchomionym skrypcie.

#### Typy zmiennych eksportowanych

Silnik na chwilę obecną wspiera następujące typy zmiennych eksportowanych:

- `number`, liczba (`int` lub `float`)
- `string`, tekst
- `bool`, wartość logiczna (`true` lub `false`)
- `vec3`, wektor 3D (obiekt z atrybutami `.x`, `.y`, `.z`)

#### Tworzenie zmiennych eksportowanych

Aby utworzyć zmienną eksportowaną, należy skorzystać z funkcji `export()` na górze edytowanego skryptu (poza blokiem funkcji).
<br><br>
<b>Przykład:</b>
<br>
- `export("liczba", 1.5)` - tworzy zmienną eksportowaną o nazwie `liczba` i wartości `1.5` typu `number`.

- `export("tekst", "Hello world!")` - tworzy zmienną eksportowaną o nazwie `tekst` i wartości `Hello world!` typu `string`.
- `export("zmienna", true)` - tworzy zmienną eksportowaną o nazwie `zmienna` i wartości `true` typu `bool`.
- `export("wektor", 1, 2, 3)` - tworzy zmienną eksportowaną o nazwie `wektor` i wartości `vec3(1.0, 2.0, 3.0)` typu `vec3`.

### Klasy udostępniające funkcje


> [!WARNING]
> Klasy oznaczone gwiazdką (*) będą działać, jeśli GameObject zawierający skrypt posiada dany komponent.

<details>
  <summary>
    <b>dBengine - funkcje wbudowane</b>
  </summary>

  `dBengine:Quit()` - zamyka silnik.
</details>

<details>
  <summary>
    <b>GameObject - funkcje udostępnione przez klasę GameObject</b>
  </summary>

<b>Pozycja lokalna</b>

```GameObject:GetLocalPosition()``` - zwraca obiekt, który zawiera atrybuty (float) .x, .y, .z.
  
```GameObject:SetLocalPosition(x, y, z)``` - ustawia pozycję lokalną obiektu. Przyjmuje argumenty typu float.

<b>Rotacja</b>

``GameObject:GetRotation()`` - zwraca obiekt, który zawiera atrybuty (float) .x, .y, .z.

``GameObject:SetRotation(x, y, z)`` - ustawia rotację Eulera. Przyjmuje argumenty typu float.

<b>Skala</b>

`GameObject:GetScale()` - zwraca obiekt, który zawiera atrybuty (float) .x, .y, .z.

`GameObject:SetScale(x, y, z)` - ustawia skalę obiektu. Przyjmuje argumenty typu float.


</details>

<details>
    <summary>
        <b>Input - funkcje związane z wejściem</b>
    </summary>

<b>Mysz</b>

`Input::IsCursorLocked()` - `true`, jeśli kursor jest przechwytywany przez okno, w przeciwnym wypadku`false`.

`Input:SetCursorLocked(mode)` - ustawia tryb blokady kursora. Przyjmuje argument `mode` typu `bool`.

`Input:GetMousePosition()` - zwraca pozycję kursora - obiekt z atrybutami `.x` oraz `.y`.
    

<b>Akcje oraz przyciski</b>

`Input:IsActionJustPressed(actionName)` - zwraca `true` w chwili, gdy dany klawisz przypisany do akcji został wciśnięty. Przyjmuje argument `actionName` typu `string`.

`Input:IsActionJustReleased(actionName)` - zwraca `true` w chwili, gdy dany klawisz przypisany do akcji został puszczony. Przyjmuje argument `actionName` typu `string`.

`Input:IsActionPressed(actionName)` - zwraca `true`, jeśli klawisz przypisany do danej akcji jest wciśnięty, natomiast `false`, gdy nie jest. Przyjmuje argument `actionName` typu `string`.
</details>

<details>

<summary>
    <b>*Camera - funkcje związane z kamerą</b>    
</summary>

`Camera:GetFront()` - zwraca wektor <i>front</i> kamery. Obiekt posiada atrybuty `.x`, `.y` oraz `.z`.

</details>

<details>
<summary>
    <b>*TextRenderer - funkcje do manipulowania tekstem 2D</b>
</summary>

`TextRenderer:SetText(text)` - ustawia atrybut `text` na komponencie `TextRenderer`.

</details>


<details>
<summary>
    <b>*Animator - funkcje umożliwiające korzystanie z animatora</b>
</summary>

<i>In the works...</i>

</details>

<details>
<summary>
    <b>*ParticleSystem - funkcje umożliwiające zarządzanie emisją cząstek</b>
</summary>

<i>In the works...</i>

</details>
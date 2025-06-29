-- TypeWriter.lua
local TypeWriter = {}
TypeWriter.__index = TypeWriter

--- Tworzy instancj� typewritera
function TypeWriter:new(textObj, fullText, cps, onDone)
    local o = setmetatable({
        textObj   = textObj,
        fullText  = fullText or "",
        cps       = cps or 30,
        onDone    = onDone,
        elapsed   = 0,
        index     = 0,
        done      = false,
    }, self)
    o.textObj:SetText("")
    return o
end

--- Ustawia nowy tekst do wy�wietlenia i resetuje stan
function TypeWriter:setText(fullText, cps, onDone)
    self.fullText = fullText or self.fullText
    if cps then self.cps = cps end
    if onDone then self.onDone = onDone end
    self.elapsed = 0
    self.index   = 0
    self.done    = false
    self.textObj:SetText("")
end

--- Nale�y wo�a� w ka�dej klatce
function TypeWriter:update(deltaTime)
    if self.done then return end

    -- Aktualizacja czasu i obliczenie docelowego indeksu
    self.elapsed = self.elapsed + deltaTime
    local target = math.floor(self.elapsed * self.cps)
    local len = #self.fullText
    if target >= len then
        target = len
        self.done = true
    end

    -- Wy�wietl nowe znaki, je�li s�
    if target > self.index then
        self.index = target
        self.textObj:SetText(self.fullText:sub(1, self.index))
    end

    -- Je�li zako�czono pisanie, wywo�aj callback raz
    if self.done and self.onDone then
        local cb = self.onDone
        self.onDone = nil
        cb()
    end
end

return TypeWriter

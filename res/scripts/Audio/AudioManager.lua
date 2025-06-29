local audioSource

function connectSignals()
    signals.Timer_OnTimeout:connect(uuid .. "_onTimeout", function()

    end, "onBeat")
end

function disconnectSignals()
    signals.Timer_OnTimeout:disconnect(uuid .. "_onTimeout")
end


function onReady()
    if not audioSource then
        audioSource = self():GetComponent("AudioSource")
    end

end
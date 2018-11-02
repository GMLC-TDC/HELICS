function v = helics_state_pending_init()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856880);
  end
  v = vInitialized;
end

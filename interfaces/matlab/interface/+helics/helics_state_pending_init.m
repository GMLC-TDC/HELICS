function v = helics_state_pending_init()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 122);
  end
  v = vInitialized;
end

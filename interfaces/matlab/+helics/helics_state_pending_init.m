function v = helics_state_pending_init()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535379);
  end
  v = vInitialized;
end

function v = helics_state_startup()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 100);
  end
  v = vInitialized;
end

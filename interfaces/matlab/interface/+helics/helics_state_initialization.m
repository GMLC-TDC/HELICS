function v = helics_state_initialization()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 104);
  end
  v = vInitialized;
end

function v = helics_state_initialization()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 21);
  end
  v = vInitialized;
end

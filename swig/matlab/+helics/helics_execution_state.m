function v = helics_execution_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 20);
  end
  v = vInitialized;
end

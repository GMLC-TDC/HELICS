function v = helics_execution_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 18);
  end
  v = vInitialized;
end

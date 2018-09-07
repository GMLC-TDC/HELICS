function v = helics_execution_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876572);
  end
  v = vInitialized;
end

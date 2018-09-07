function v = helics_execution_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783855);
  end
  v = vInitialized;
end

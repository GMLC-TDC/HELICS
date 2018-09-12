function v = helics_execution_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230874);
  end
  v = vInitialized;
end

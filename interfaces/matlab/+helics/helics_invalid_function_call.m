function v = helics_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183041);
  end
  v = vInitialized;
end

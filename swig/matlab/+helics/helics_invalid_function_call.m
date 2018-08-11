function v = helics_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 8);
  end
  v = vInitialized;
end

function v = helics_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 7);
  end
  v = vInitialized;
end

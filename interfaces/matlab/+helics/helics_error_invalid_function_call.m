function v = helics_error_invalid_function_call()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183076);
  end
  v = vInitialized;
end

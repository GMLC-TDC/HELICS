function v = HELICS_ERROR_INVALID_FUNCTION_CALL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 73);
  end
  v = vInitialized;
end

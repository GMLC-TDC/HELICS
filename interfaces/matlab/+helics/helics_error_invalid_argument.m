function v = helics_error_invalid_argument()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183082);
  end
  v = vInitialized;
end

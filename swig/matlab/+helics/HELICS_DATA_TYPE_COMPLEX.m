function v = HELICS_DATA_TYPE_COMPLEX()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 42);
  end
  v = vInitialized;
end

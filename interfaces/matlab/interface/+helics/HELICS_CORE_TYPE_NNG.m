function v = HELICS_CORE_TYPE_NNG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 9);
  end
  v = vInitialized;
end

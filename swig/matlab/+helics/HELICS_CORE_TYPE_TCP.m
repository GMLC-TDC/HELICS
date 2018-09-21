function v = HELICS_CORE_TYPE_TCP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107625);
  end
  v = vInitialized;
end

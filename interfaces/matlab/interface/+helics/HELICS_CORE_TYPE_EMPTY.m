function v = HELICS_CORE_TYPE_EMPTY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 15);
  end
  v = vInitialized;
end

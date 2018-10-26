function v = HELICS_CORE_TYPE_INTERPROCESS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783872);
  end
  v = vInitialized;
end

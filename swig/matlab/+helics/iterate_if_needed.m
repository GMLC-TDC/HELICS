function v = iterate_if_needed()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 12);
  end
  v = vInitialized;
end
